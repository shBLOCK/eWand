#pragma once

void power_stage_init();

//TODO maybe not expose this (to be) internal function
void power_stage_set_output_params(uint16_t top, uint16_t cc);

void power_stage_set_enable(bool enable);
