#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "utils/utils.h"

#include "power_stage.h"

#define PS_PIN_INV_INA 29
#define PS_PIN_INV_INB 27
#define PS_PIN_INV_ENA 28
#define PS_PIN_INV_ENB 26

// debug: output to LED
#if false
#undef PS_PIN_INV_INA
#define PS_PIN_INV_INA 25
#endif

const uint PS_PINS_INV_IN[] = {PS_PIN_INV_INA, PS_PIN_INV_INB};
const uint PS_PINS_INV_EN[] = {PS_PIN_INV_ENA, PS_PIN_INV_ENB};

static uint PS_PWM_SLICE[2];
static enum pwm_chan PS_PWM_CHAN[2];
static uint PS_DMA_CTRL;
static uint PS_DMA_CTRLRST;
static uint PS_DMA_DATA;

typedef struct __packed_aligned {
    uint16_t cc;
    uint16_t top;
} output_params;
static volatile output_params pwm_cfg; // to ensure atomic write from MCU
static volatile output_params pwm_cfg_buf; // double buffering, DMA reads from this

static uint8_t dummy = 0;
const static uint32_t ZERO32 = 0;
const static uint32_t CTRLRST_TRANS_COUNT_TRIG = 1;
static volatile struct __packed_aligned {
    volatile const void* read_addr;
    volatile void* write_addr;
    uint32_t trans_count;
    uint32_t ctrl;
} control_blocks[] = { // ctrl values to be configured in power_stage_init()
    {&pwm_cfg, &pwm_cfg_buf, 1, 0}, // copy pwm_cfg to pwm_cfg_buf atomically
    {&pwm_cfg_buf.top, NULL, 1, 0}, // configure PWM A top
    {&pwm_cfg_buf.top, NULL, 1, 0}, // configure PWM B top
    {&pwm_cfg_buf.cc, NULL, 1, 0}, // configure PWM A cc
    {&ZERO32, NULL, 1, 0}, // configure PWM B cc
    {&dummy, &dummy, 1, 0}, // wait for the next PWM wrap
    {&ZERO32, NULL, 1, 0}, // configure PWM A cc
    {&pwm_cfg_buf.cc, NULL, 1, 0}, // configure PWM B cc
    {&CTRLRST_TRANS_COUNT_TRIG, NULL, 1, 0}, // trigger PS_DMA_CTRLRST
};
const static volatile void* CTRL_READ_ADDR_TRIG = &control_blocks[0];

void power_stage_init() {
    printf("Initializing PowerStage...\n");

    // INH Pins: GPIO
    for (int i = 0; i < 2; ++i) {
        const uint pin = PS_PINS_INV_EN[i];
        gpio_init(pin);
        gpio_put(pin, false);
        gpio_strong_output(pin);
    }

    // IN Pins: PWM
    uint pwm_en_mask = 0;
    for (int i = 0; i < 2; ++i) {
        const uint pin = PS_PINS_INV_IN[i];
        gpio_set_function(pin, GPIO_FUNC_PWM);
        gpio_strong_output(pin);
        const uint slice = pwm_gpio_to_slice_num(pin);
        PS_PWM_SLICE[i] = slice;
        const enum pwm_chan chan = pwm_gpio_to_channel(pin);
        PS_PWM_CHAN[i] = chan;

        pwm_config cfg = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&cfg, 1);
        pwm_init(slice, &cfg, false);
        pwm_set_chan_level(slice, chan, 0);
        pwm_en_mask |= 1 << slice;
    }
    // Enable the two PWM in sync
    pwm_set_mask_enabled(pwm_en_mask);

    power_stage_set_output_params(0xFFFF - 1, 0);


    // PWM DMAs
    // Note: PWM hardware has double-buffering,
    //       so we only need to finish reconfiguring the two PWM slices before the next wrap.
    {
        PS_DMA_CTRL = dma_claim_unused_channel(true);
        PS_DMA_CTRLRST = dma_claim_unused_channel(true);
        PS_DMA_DATA = dma_claim_unused_channel(true);

        // setup control blocks
        {
            // buffer pwm_cfg value
            dma_channel_config cfg = dma_channel_get_default_config(PS_DMA_DATA);
            channel_config_set_chain_to(&cfg, PS_DMA_CTRL);
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, true);
            control_blocks[0].ctrl = channel_config_get_ctrl_value(&cfg);

            // configure PWM
            cfg = dma_channel_get_default_config(PS_DMA_DATA);
            channel_config_set_chain_to(&cfg, PS_DMA_CTRL);
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, true);
            for (int i = 0; i < 2; ++i) {
                const uint base_index = 1;
                control_blocks[base_index + i].write_addr = &pwm_hw->slice[PS_PWM_SLICE[i]].top;
                control_blocks[base_index + i].ctrl = channel_config_get_ctrl_value(&cfg);
            }
            for (int base_i = 0; base_i < 2; ++base_i) {
                const uint bases[] = {3, 6};
                const uint base_index = bases[base_i];
                for (int i = 0; i < 2; ++i) {
                    control_blocks[base_index + i].write_addr = (void*) &pwm_hw->slice[PS_PWM_SLICE[i]].cc + 2 * PS_PWM_CHAN[i];
                    control_blocks[base_index + i].ctrl = channel_config_get_ctrl_value(&cfg);
                }
            }

            // wait for the next PWM wrap
            cfg = dma_channel_get_default_config(PS_DMA_DATA);
            channel_config_set_chain_to(&cfg, PS_DMA_CTRL);
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
            channel_config_set_dreq(&cfg, pwm_get_dreq(PS_PWM_SLICE[0]));
            control_blocks[5].ctrl = channel_config_get_ctrl_value(&cfg);

            // trigger PS_DMA_CTRLRST
            cfg = dma_channel_get_default_config(PS_DMA_DATA);
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, true);
            control_blocks[count_of(control_blocks) - 1].write_addr = &dma_hw->ch[PS_DMA_CTRLRST].al1_transfer_count_trig;
            control_blocks[count_of(control_blocks) - 1].ctrl = channel_config_get_ctrl_value(&cfg);
        }

        // master (control) DMA
        {
            dma_channel_config cfg = dma_channel_get_default_config(PS_DMA_CTRL);
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, true);
            channel_config_set_ring(&cfg, true, 4); // 16 byte, 4 words
            dma_channel_configure(
                PS_DMA_CTRL,
                &cfg,
                &dma_hw->ch[PS_DMA_DATA].read_addr, // write to the 4 alt0 regs
                NULL, // configured by PS_DMA_CTRLRST
                4,
                false
            );
        }

        // control reset (CTRLRST) DMA
        {
            dma_channel_config cfg = dma_channel_get_default_config(PS_DMA_CTRLRST);
            channel_config_set_dreq(&cfg, pwm_get_dreq(PS_PWM_SLICE[0]));
            channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
            channel_config_set_read_increment(&cfg, false);
            channel_config_set_write_increment(&cfg, false);
            dma_channel_configure(
                PS_DMA_CTRLRST,
                &cfg,
                &dma_hw->ch[PS_DMA_CTRL].al3_read_addr_trig,
                &CTRL_READ_ADDR_TRIG,
                1,
                true // start the entire DMA contraption
            );
        }
    }
}

void power_stage_set_output_params(const uint16_t top, const uint16_t cc) {
    const volatile output_params tmp = {cc, top};
    pwm_cfg = tmp; // TODO: better ways for atomic write?
}

void power_stage_set_enable(const bool enable) {
    for (int i = 0; i < count_of(PS_PINS_INV_EN); ++i) {
        gpio_put(PS_PINS_INV_EN[i], enable);
    }
}
