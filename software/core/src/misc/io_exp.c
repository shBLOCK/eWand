#include "pico/stdlib.h"
#include "misc/i2c_bus.h"

#include "io_exp.h"

void io_exp_init() {
    gpio_init(IO_EXP_PIN_INT);
    gpio_set_dir(IO_EXP_PIN_INT, GPIO_IN);
    gpio_set_pulls(IO_EXP_PIN_INT, true, false);
    reg_output = i2c_bus_read_reg8(IO_EXP_ADDR, 0x01);
    reg_inv = i2c_bus_read_reg8(IO_EXP_ADDR, 0x02);
    reg_dir = i2c_bus_read_reg8(IO_EXP_ADDR, 0x03);
    //TODO setup IRQ
}
