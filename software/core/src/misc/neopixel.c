#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

#include "neopixel.pio.h"

#include "neopixel.h"

#define NEOPIXEL_PIN 24
#define NEOPIXEL_FREQ 800000
#define NEOPIXEL_PIO pio1
static uint NEOPIXEL_SM;
#define NEOPIXEL_LEDS 6
#define NEOPIXEL_BITS neopixel_PULL_THRESHOLD
uint32_t neopixel_buf[NEOPIXEL_LEDS];

void neopixel_init() {
    pio_gpio_init(NEOPIXEL_PIO, NEOPIXEL_PIN);
    NEOPIXEL_SM = pio_claim_unused_sm(NEOPIXEL_PIO, true);
    const uint offset = pio_add_program(NEOPIXEL_PIO, &neopixel_program);
    pio_sm_set_consecutive_pindirs(NEOPIXEL_PIO, NEOPIXEL_SM, NEOPIXEL_PIN, 1, true);

    pio_sm_config c = neopixel_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, NEOPIXEL_PIN);
    sm_config_set_out_shift(&c, false, true, NEOPIXEL_BITS);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    const int cycles_per_bit = neopixel_T1 + neopixel_T2 + neopixel_T3;
    const float div = (float) clock_get_hz(clk_sys) / (float) (NEOPIXEL_FREQ * cycles_per_bit);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(NEOPIXEL_PIO, NEOPIXEL_SM, offset, &c);
    pio_sm_set_enabled(NEOPIXEL_PIO, NEOPIXEL_SM, true);
}

void neopixel_upload() {
    pio_sm_put_blocking(NEOPIXEL_PIO, NEOPIXEL_SM, (NEOPIXEL_LEDS * NEOPIXEL_BITS) << 8);
    for (int i = 0; i < NEOPIXEL_LEDS; ++i) {
        pio_sm_put_blocking(NEOPIXEL_PIO, NEOPIXEL_SM, neopixel_buf[i] << (32 - NEOPIXEL_BITS));
    }
}
