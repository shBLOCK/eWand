/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"

#include "power_stage/power_stage.h"

int main() {
    stdio_init_all();
    power_stage_init();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
