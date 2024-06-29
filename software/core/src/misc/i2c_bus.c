#include "pico/stdlib.h"
#include "utils/utils.h"

#include "i2c_bus.h"

#define I2C_BUS_PIN_SDA 14
#define I2C_BUS_PIN_SCL 15

void i2c_bus_init() {
    i2c_init(I2C_BUS_INST, 400 * 1000);
    gpio_set_function(I2C_BUS_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_BUS_PIN_SCL, GPIO_FUNC_I2C);
    gpio_strong_output(I2C_BUS_PIN_SDA);
    gpio_strong_output(I2C_BUS_PIN_SCL);
}

