/**
* INA226 Power Monitor (on VBUS)
* https://www.ti.com/lit/ds/symlink/ina226.pdf
*/

#include <math.h>
#include "pico/stdlib.h"
#include "misc/i2c_bus.h"

#include "power_monitor.h"

#define ADDR 0x40
#define REG_CONFIGURATION 0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE 0x02
#define REG_POWER 0x03
#define REG_CURRENT 0x04
#define REG_CALIBRATION 0x05
#define REG_MASK_AND_STATUS 0x06
#define REG_ALERT_LIMIT 0x07
#define REG_MANUFACTRUER_ID 0xFE
#define REG_DIE_ID 0xFF

enum power_monitor_average_mode {
    POWER_MONITOR_AVG_1 = 0,
    POWER_MONITOR_AVG_4 = 1,
    POWER_MONITOR_AVG_16 = 2,
    POWER_MONITOR_AVG_64 = 3,
    POWER_MONITOR_AVG_128 = 4,
    POWER_MONITOR_AVG_256 = 5,
    POWER_MONITOR_AVG_512 = 6,
    POWER_MONITOR_AVG_1024 = 7
};

enum power_monitor_conversion_time {
    POWER_MONITOR_CT_140us = 0,
    POWER_MONITOR_CT_204us = 1,
    POWER_MONITOR_CT_332us = 2,
    POWER_MONITOR_CT_588us = 3,
    POWER_MONITOR_CT_1100us = 4,
    POWER_MONITOR_CT_2116us = 5,
    POWER_MONITOR_CT_4156us = 6,
    POWER_MONITOR_CT_8244us = 7
};

#define AVG_MODE POWER_MONITOR_AVG_1
#define BUS_VOLTAGE_CONVERSION_TIME POWER_MONITOR_CT_1100us
#define SHUNT_VOLTAGE_CONVERSION_TIME POWER_MONITOR_CT_1100us
#define BUS_VOLTAGE_EN true
#define SHUNT_VOLTAGE_EN true
#define CONTINOUS_CONVERSION true

#define BUS_VOLTAGE_LSB 1.25e-3f
#define SHUNT_RESISTANCE 10e-3f
#define MAX_CURRENT 10.0f // amps

static uint16_t reg_calibration;
static float current_lsb;
static float power_lsb;

void power_monitor_init() {
    uint16_t reg_config = 0;
    reg_config |= (uint16_t) AVG_MODE << 9;
    reg_config |= (uint16_t) BUS_VOLTAGE_CONVERSION_TIME << 6;
    reg_config |= (uint16_t) SHUNT_VOLTAGE_CONVERSION_TIME << 3;
    reg_config |= bool_to_bit(CONTINOUS_CONVERSION) << 2;
    reg_config |= bool_to_bit(BUS_VOLTAGE_EN) << 1;
    reg_config |= bool_to_bit(SHUNT_VOLTAGE_EN) << 0;
    i2c_bus_write_reg16(ADDR, REG_CONFIGURATION, reg_config);

    reg_calibration = (uint16_t) floorf(167.77216f / (MAX_CURRENT * SHUNT_RESISTANCE));
    i2c_bus_write_reg16(ADDR, REG_CALIBRATION, reg_calibration);

    current_lsb = 0.00512f / ((float) reg_calibration * SHUNT_RESISTANCE);
    power_lsb = 20000.0f * BUS_VOLTAGE_LSB * current_lsb;
}

float power_monitor_get_voltage() {
    return (float) i2c_bus_read_reg16(ADDR, REG_BUS_VOLTAGE) * BUS_VOLTAGE_LSB;
}

float power_monitor_get_current() {
    const uint16_t reg = i2c_bus_read_reg16(ADDR, REG_CURRENT);
    return (float) *(int16_t*)&reg * current_lsb;
}

float power_monitor_get_power() {
    return (float) i2c_bus_read_reg16(ADDR, REG_POWER) * power_lsb;
}
