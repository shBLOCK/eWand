#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "power_stage.pio.h"

#include "power_stage.h"

#define PS_PIN_INV_INA 29
#define PS_PIN_INV_INB 27
#define PS_PIN_INV_INHA 28
#define PS_PIN_INV_INHB 26
const uint PS_PINS_INV_IN[] = {PS_PIN_INV_INA, PS_PIN_INV_INB};
const uint PS_PINS_INV_INH[] = {PS_PIN_INV_INHA, PS_PIN_INV_INHB};

#define PS_PIO pio0
uint PS_PIO_SMS[2];

void power_stage_init() {
    printf("Initializing PowerStage...\n");

    for (int i = 0; i < 2; ++i) {
        const uint pin = PS_PINS_INV_INH[i];
        gpio_init(pin);
        gpio_disable_pulls(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_slew_rate(pin, GPIO_SLEW_RATE_FAST);
    }

    uint offset = pio_add_program(PS_PIO, &power_stage_program);
    for (int i = 0; i < 2; ++i) {
        PS_PIO_SMS[i] = pio_claim_unused_sm(PS_PIO, true);
        power_stage_pio_program_init(PS_PIO_SMS[i], PS_PINS_INV_IN[i], offset);
    }
}

static inline void power_stage_pio_program_init(uint sm, uint pin, uint offset) {
    pio_gpio_init(PS_PIO, pin);
    pio_sm_config cfg = power_stage_program_get_default_config(offset);
    sm_config_set_out_pins(&cfg, pin, 1);
    sm_config_set_out_shift(&cfg, true, true, 16);
    sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_TX);
    pio_sm_init(PS_PIO, sm, offset, &cfg);
}
