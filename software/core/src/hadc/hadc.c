#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "pico/sync.h"
#include "misc/io_exp.h"

#include "hadc.pio.h"

#include "hadc.h"

#define HADC_BITS 12
#define HADC_PIN_CLK 0
#define HADC_PIN_DATA 1
#define HADC_PIN_OTR 13 //TODO: handle HADC OTR
#define HADC_EXPPIN_PDWN 6
#define HADC_PIO pio1
static uint HADC_SM;
static uint HADC_DMA_ACC[2]; // chained to each other
#define HADC_DMA_IRQ DMA_IRQ_0
static uint32_t dma_dummy_dst;
static uint16_t snapshot[16384];
static uint64_t master_accumulator;

static void hadc_dma_sum_handler() {
    const uint chan = dma_channel_is_busy(HADC_DMA_ACC[0]) ? HADC_DMA_ACC[1] : HADC_DMA_ACC[0];
    const uint32_t acc = dma_sniffer_get_data_accumulator();
    dma_sniffer_set_data_accumulator(0);
    dma_irqn_acknowledge_channel(HADC_DMA_IRQ, chan);
    master_accumulator += acc;
    printf("HADC DMA %u: ACC %lu\n", chan, acc);
}

void hadc_init() {
    printf("Initializing High Speed ADC...\n");

    io_exp_put(HADC_EXPPIN_PDWN, 0);
    io_exp_set_dir(HADC_EXPPIN_PDWN, GPIO_OUT);
    sleep_ms(3);

    // PIO
    {
        HADC_SM = pio_claim_unused_sm(HADC_PIO, true);
        uint offset = pio_add_program(HADC_PIO, &hadc_program);

        gpio_strong_output(HADC_PIN_CLK);
        pio_gpio_init(HADC_PIO, HADC_PIN_CLK);
        pio_sm_set_consecutive_pindirs(HADC_PIO, HADC_SM, HADC_PIN_CLK, 1, true);
        for (int i = 0; i < HADC_BITS; ++i) {
            const uint pin = HADC_PIN_DATA + i;
            gpio_set_dir(pin, GPIO_IN);
            pio_gpio_init(HADC_PIO, pin);
        }
        pio_sm_set_consecutive_pindirs(HADC_PIO, HADC_SM, HADC_PIN_DATA, HADC_BITS, false);
        gpio_set_dir(HADC_PIN_OTR, GPIO_IN);
        pio_sm_set_consecutive_pindirs(HADC_PIO, HADC_SM, HADC_PIN_OTR, 1, false);

        pio_sm_config cfg = hadc_program_get_default_config(offset);
        sm_config_set_in_shift(&cfg, true, true, 32);
        sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_RX);
        sm_config_set_clkdiv_int_frac(&cfg, 1, 0);
        sm_config_set_sideset_pins(&cfg, HADC_PIN_CLK);
        sm_config_set_in_pins(&cfg, HADC_PIN_DATA);
        pio_sm_init(HADC_PIO, HADC_SM, offset + hadc_offset_entry, &cfg);
    }

    // Integrating DMA
    {
        for (int i = 0; i < 2; ++i) {
            HADC_DMA_ACC[i] = dma_claim_unused_channel(true);
        }

        for (int i = 0; i < 2; ++i) {
            const uint chan = HADC_DMA_ACC[i];
            dma_channel_config cfg = dma_get_channel_config(chan);
            channel_config_set_dreq(&cfg, pio_get_dreq(HADC_PIO, HADC_SM, false));
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
            channel_config_set_read_increment(&cfg, false);
            channel_config_set_write_increment(&cfg, false);
            channel_config_set_sniff_enable(&cfg, true);
            channel_config_set_chain_to(&cfg, HADC_DMA_ACC[!i]);
            
            dma_sniffer_set_data_accumulator(0);
            dma_sniffer_enable(chan, DMA_SNIFF_CTRL_CALC_VALUE_SUM, true);
            dma_channel_configure(
                chan,
                &cfg,
                &dma_dummy_dst,
                &HADC_PIO->rxf[HADC_SM],
                (1 << (32 - 1)) / (1 << (12 - 1)),
                false
            );
            dma_irqn_set_channel_enabled(HADC_DMA_IRQ, chan, true);
            irq_set_exclusive_handler(HADC_DMA_IRQ, hadc_dma_sum_handler);
            irq_set_enabled(HADC_DMA_IRQ, true);
        }
    }

    pio_sm_set_enabled(HADC_PIO, HADC_SM, true);

    // Discard the first 8 samples (1 invalid data + 7 pipeline delay)
    for (int i = 0; i < 8; ++i) {
        pio_sm_get_blocking(HADC_PIO, HADC_SM);
    }

    dma_channel_start(HADC_DMA_ACC[0]);
}

void hadc_set_enable(const bool enable) {
    io_exp_put(HADC_EXPPIN_PDWN, !enable);
    if (enable) sleep_ms(3);
}

void hadc_capture_snapshot() {
    uint chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_get_channel_config(chan);
    channel_config_set_dreq(&cfg, pio_get_dreq(HADC_PIO, HADC_SM, false));
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    dma_channel_configure(
        chan,
        &cfg,
        &snapshot,
        &HADC_PIO->rxf[HADC_SM] + 2, // read 16 LSBs
        count_of(snapshot),
        true
    );
}
