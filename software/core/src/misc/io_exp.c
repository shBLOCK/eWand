#include "pico/stdlib.h"
#include "misc/i2c_bus.h"

#include "io_exp.h"

void io_exp_init() {
    reg_output = i2c_bus_read_reg8(IO_EXP_ADDR, 0x01, false);
    reg_inv = i2c_bus_read_reg8(IO_EXP_ADDR, 0x02, false);
    reg_dir = i2c_bus_read_reg8(IO_EXP_ADDR, 0x03, false);
    //TODO setup IRQ
}
