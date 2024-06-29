#ifndef EWANDCORE_IO_EXP_H
#define EWANDCORE_IO_EXP_H

#include "misc/i2c_bus.h"
#include "utils/utils.h"

#define IO_EXP_PIN_INT 25
#define IO_EXP_ADDR 0x20

void io_exp_init();

static uint8_t reg_output, reg_inv, reg_dir;

static inline bool io_exp_get(uint pin) {
    return bit_get(i2c_bus_read_reg8(IO_EXP_ADDR, 0x00, false), pin);
}

static inline void io_exp_put(uint pin, bool value) {
    reg_output = bit_put(reg_output, pin, value);
    i2c_bus_write_reg8(IO_EXP_ADDR, 0x01, reg_output, false);
}

static inline void io_exp_set_inv(uint pin, bool inv) {
    reg_inv = bit_put(reg_inv, pin, inv);
    i2c_bus_write_reg8(IO_EXP_ADDR, 0x02, reg_inv, false);
}

static inline void io_exp_set_dir(uint pin, bool output) {
    reg_dir = bit_put(reg_dir, pin, output);
    i2c_bus_write_reg8(IO_EXP_ADDR, 0x03, reg_dir, false);
}

#endif //EWANDCORE_IO_EXP_H
