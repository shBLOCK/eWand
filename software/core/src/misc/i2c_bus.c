#include "pico/stdlib.h"
#include "utils/utils.h"

#include "i2c_bus.h"

#include <stdio.h>

#define I2C_BUS_PIN_SDA 14
#define I2C_BUS_PIN_SCL 15
#define I2C_BUS_FREQ (100 * 1000)

void i2c_bus_init() {
    i2c_init(I2C_BUS_INST, I2C_BUS_FREQ);
    gpio_set_function(I2C_BUS_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_BUS_PIN_SCL, GPIO_FUNC_I2C);
    gpio_strong_output(I2C_BUS_PIN_SDA);
    gpio_strong_output(I2C_BUS_PIN_SCL);
}

void i2c_bus_debug_scan() {
    /**
     * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
     *
     * SPDX-License-Identifier: BSD-3-Clause
     */

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        uint8_t tmp = 1;
        if ((addr & 0x78) == 0 || (addr & 0x78) == 0x78) // reserved
            ret = PICO_ERROR_GENERIC;
        else
            //TODO: should use a 0-length write here
            i2c_write_blocking(I2C_BUS_INST, addr, &tmp, 1, true);
            ret = i2c_read_blocking(I2C_BUS_INST, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
}
