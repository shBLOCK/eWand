/**
 * TPS55288 Buck/Boost Converter
 * https://www.ti.com/lit/ds/symlink/tps55288-q1.pdf
 *
 * Refer to: /hardware/etc/TPS55288 Design Calculation V1.0.xlsx
 */

#include "pico/stdlib.h"
#include "pico/float.h"
#include "misc/i2c_bus.h"

#include "dcdc.h"

#include <utils/utils.h>

#define ADDR 0x74

#define REG_VREF_L 0x00
#define REG_VREF_H 0x01
#define REG_I_LIMIT 0x02
#define REG_VOUT_SR 0x03
#define REG_VOUT_FS 0x04
#define REG_CDC 0x05
#define REG_MODE 0x06
#define REG_STATUS 0x07

#define REG_MODE_VALUE_NO_OE 0x39

void dcdc_init() {
    i2c_bus_write_reg8(ADDR, REG_I_LIMIT, 0xE4);
    i2c_bus_write_reg8(ADDR, REG_VOUT_SR, 0x01);
    i2c_bus_write_reg8(ADDR, REG_VOUT_FS, 0x03);
    i2c_bus_write_reg8(ADDR, REG_CDC, 0xE0);
    i2c_bus_write_reg8(ADDR, REG_MODE, REG_MODE_VALUE_NO_OE);
}

#define FB_RATIO 0.0564f

void dcdc_set_output_enable(const bool enable) {
    i2c_bus_write_reg8(ADDR, REG_MODE, REG_MODE_VALUE_NO_OE | (bool_to_bit(enable) << 7));
}

void dcdc_set_voltage(const float output) {
    float fref = output * FB_RATIO;
    if (!isfinite(fref)) panic("Invalid TPS55288 DCDC voltage setting: %f", output);
    if (fref < 45e-3f) fref = 45e-3f;
    if (fref > 1200e-3f) fref = 1200e-3f;
    uint16_t ref = (uint16_t) roundf((fref - 45e-3f) / 1.129e-3f);
    if (ref & 0xFC00) ref = 0x3FF;

    i2c_bus_write_reg8(ADDR, REG_VREF_L, ref & 0xFF);
    i2c_bus_write_reg8(ADDR, REG_VREF_H, (ref >> 8) & 0b11);
}

#define DCDC_ERR_SHORT_CIRCUIT (1 << 2)
#define DCDC_ERR_OVER_CURRENT (1 << 1)
#define DCDC_ERR_OVER_VOLTAGE (1 << 0)

uint8_t dcdc_get_err() {
    return i2c_bus_read_reg8(ADDR, REG_STATUS) >> 5;
}

enum dcdc_mode {
    DCDC_MODE_BOOST = 0b00,
    DCDC_MODE_BUCK = 0b01,
    DCDC_MODE_BUCK_BOOST = 0b10,
    DCDC_MODE_INVALID = 0b11
};

enum dcdc_mode dcdc_get_mode() {
    return i2c_bus_read_reg8(ADDR, REG_STATUS) & 0b11;
}
