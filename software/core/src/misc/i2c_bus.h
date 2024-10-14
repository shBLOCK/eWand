#pragma once

#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/sync.h"

#define I2C_BUS_INST i2c1

extern critical_section_t i2c_bus_critical_section;

void i2c_bus_init();

static inline void i2c_bus_acquire() {
    critical_section_enter_blocking(&i2c_bus_critical_section);
}

static inline void i2c_bus_release() {
    critical_section_exit(&i2c_bus_critical_section);
}

static inline void i2c_bus_write_reg8(const uint8_t addr, const uint8_t reg, const uint8_t value) {
    i2c_bus_acquire();
    const uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_BUS_INST, addr, buf, 2, false);
    i2c_bus_release();
}

static inline uint8_t i2c_bus_read_reg8(const uint8_t addr, const uint8_t reg) {
    i2c_bus_acquire();
    i2c_write_blocking(I2C_BUS_INST, addr, &reg, 1, true);
    uint8_t result;
    i2c_read_blocking(I2C_BUS_INST, addr, &result, 1, false);
    i2c_bus_release();
    return result;
}

static inline void i2c_bus_write_reg16(const uint8_t addr, const uint8_t reg, const uint16_t value) {
    i2c_bus_acquire();
    const uint8_t buf[] = {reg, value >> 8, value & 0xFF};
    i2c_write_blocking(I2C_BUS_INST, addr, buf, 3, false);
    i2c_bus_release();
}

static inline uint16_t i2c_bus_read_reg16(const uint8_t addr, const uint8_t reg) {
    i2c_bus_acquire();
    i2c_write_blocking(I2C_BUS_INST, addr, &reg, 1, true);
    uint8_t result[2];
    i2c_read_blocking(I2C_BUS_INST, addr, result, 2, false);
    i2c_bus_release();
    return result[0] << 8 | result[1];
}

void i2c_bus_debug_scan();
