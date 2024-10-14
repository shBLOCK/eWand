/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "hardware/vreg.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/time.h"
#include "hardware/watchdog.h"

#include "misc/i2c_bus.h"
#include "misc/io_exp.h"
#include "power_stage/power_stage.h"
#include "hadc/hadc.h"
#include "misc/neopixel.h"
#include "dcdc/dcdc.h"
#include "misc/power_monitor.h"

void main_core1();

int main() {
    stdio_init_all();

    printf("CHIP_RESET: %x\n", vreg_and_chip_reset_hw->chip_reset);
    if (vreg_and_chip_reset_hw->chip_reset & (1 << 16)) {
        printf("Reset: RUN Pin\n");
    }
    if (vreg_and_chip_reset_hw->chip_reset & (1 << 8)) {
        printf("Reset: Power\n");
    }
    if (vreg_and_chip_reset_hw->chip_reset & (1 << 20)) {
        printf("Reset: Debug Port (PSM reset)\n");
    }
    if (vreg_and_chip_reset_hw->chip_reset & (1 << 24)) {
        printf("PSM_RESTART_FLAG set\n");
    }

    i2c_bus_init();
    i2c_bus_debug_scan();
    neopixel_init();
    neopixel_upload();

    multicore_reset_core1();
    multicore_launch_core1(main_core1);

    power_monitor_init();

    // Notify init done
    multicore_fifo_pop_blocking();
    multicore_fifo_push_blocking(0);

    printf("Initialization complete!\n");

    while (true) {
        printf("BUS Voltage: %.2f\n", power_monitor_get_voltage());
        printf("BUS Current: %.2f\n", power_monitor_get_current());
        sleep_ms(1000);
    }
    // uint8_t intensity = 0;
    // while (true) {
    //     neopixel_set_rgb(0, intensity, 0, 0);
    //     neopixel_set_rgb(1, 0, intensity, 0);
    //     neopixel_set_rgb(2, 0, 0, intensity);
    //     neopixel_upload();
    //     intensity++;
    //     // sleep_ms(1);
    // }
}

#define D_V 12.f

void main_core1() {
    io_exp_init();
    dcdc_init();

    // hadc_init();
    power_stage_init();

    // Notify init done
    multicore_fifo_push_blocking(0);
    multicore_fifo_pop_blocking();

    // dcdc_set_output_enable(false);
    // sleep_ms(3000);
    dcdc_set_voltage(D_V);
    dcdc_set_output_enable(true);

    power_stage_set_enable(false);
    power_stage_set_output_params(4000-1, 4000);

    while (true) {
        while (true) {
            const char in = getchar_timeout_us(0);
            if (in == PICO_ERROR_TIMEOUT)
                break;
            if (in == ';') {
                power_stage_set_enable(true);
                dcdc_set_voltage(D_V);
                dcdc_set_output_enable(true);
                printf("Output Enabled\n");
            } else if (in == '\'') {
                power_stage_set_enable(false);
                printf("Output Disabled\n");
            }
        }

        // power_stage_set_enable(true);
        // dcdc_set_voltage(D_V);
        // dcdc_set_output_enable(true);

        const uint dcdc_err = dcdc_get_err();
        if (dcdc_err) {
            printf("DCDC Error: %u\n", dcdc_get_err());
        }
        // dcdc_set_voltage(D_V);
        sleep_ms(10);
    }
}
