#ifndef EWANDCORE_I2C_BUS_H
#define EWANDCORE_I2C_BUS_H

#include "hardware/i2c.h"

#define I2C_BUS_INST i2c1

void i2c_bus_init();

static inline void i2c_bus_write_reg8(uint8_t addr, uint8_t reg, uint8_t value, bool nostop) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_BUS_INST, addr, buf, 2, nostop);
}

static inline uint8_t i2c_bus_read_reg8(uint8_t addr, uint8_t reg, bool nostop) {
    i2c_write_blocking(I2C_BUS_INST, addr, &reg, 1, true);
    uint8_t result;
    i2c_read_blocking(I2C_BUS_INST, addr, &result, 1, nostop);
    return result;
}

#endif //EWANDCORE_I2C_BUS_H
