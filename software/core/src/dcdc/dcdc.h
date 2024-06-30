#pragma once

void dcdc_init();

void dcdc_set_output_enable(bool enable);

void dcdc_set_voltage(float output);

uint8_t dcdc_get_err();

enum dcdc_mode dcdc_get_mode();
