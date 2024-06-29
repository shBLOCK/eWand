#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <stdio.h>

extern uint32_t neopixel_buf[];

void neopixel_init();

static inline void neopixel_set(const uint index, const uint32_t color) {
    neopixel_buf[index] = color;
}

static inline void neopixel_set_rgb(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    neopixel_buf[index] = (uint32_t) r << 16 | (uint32_t) g << 8 | (uint32_t) b;
}

static inline uint32_t neopixel_get(const uint index) {
    return neopixel_buf[index];
}

void neopixel_upload();

#endif //NEOPIXEL_H
