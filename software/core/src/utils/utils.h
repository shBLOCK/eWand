#pragma once

#include "pico/stdlib.h"

static inline void gpio_strong_output(uint gpio) {
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);
}

__force_inline uint bit_put(uint num, uint bit, bool value) {
    return (num & ~(1 << bit)) | (value << bit);
}

__force_inline uint bit_set(uint num, uint bit) {
    return num | (1 << bit);
}

__force_inline uint bit_clear(uint num, uint bit) {
    return num & ~(1 << bit);
}

__force_inline bool bit_get(uint num, uint bit) {
    return num & (1 << bit);
}
