#pragma once

#include <stdio.h>

#include "hardware/i2c.h"

#define I2C_BUS_INST i2c1

void i2c_bus_init();

static inline void i2c_bus_write_reg8(const uint8_t addr, const uint8_t reg, const uint8_t value) {
    const uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_BUS_INST, addr, buf, 2, false);
}

static inline uint8_t i2c_bus_read_reg8(const uint8_t addr, const uint8_t reg) {
    i2c_write_blocking(I2C_BUS_INST, addr, &reg, 1, true);
    uint8_t result;
    i2c_read_blocking(I2C_BUS_INST, addr, &result, 1, false);
    return result;
}

static inline void i2c_bus_write_reg16(const uint8_t addr, const uint8_t reg, const uint16_t value) {
    const uint8_t buf[] = {reg, value >> 8, value & 0xFF};
    i2c_write_blocking(I2C_BUS_INST, addr, buf, 3, false);
}

static inline uint16_t i2c_bus_read_reg16(const uint8_t addr, const uint8_t reg) {
    i2c_write_blocking(I2C_BUS_INST, addr, &reg, 1, true);
    uint8_t result[2];
    i2c_read_blocking(I2C_BUS_INST, addr, result, 2, false);
    return result[0] << 8 | result[1];
}

void i2c_bus_debug_scan();
