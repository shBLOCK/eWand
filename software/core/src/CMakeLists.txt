add_executable(ewand_core
        main.c
        power_stage/power_stage.c
        hadc/hadc.c
        utils/ladc.c
        misc/io_exp.c
        misc/i2c_bus.c
        misc/neopixel.c
        misc/neopixel.h
        misc/tps55288.c
        misc/tps55288.h
)

pico_generate_pio_header(ewand_core ${CMAKE_CURRENT_LIST_DIR}/hadc/hadc.pio)
pico_generate_pio_header(ewand_core ${CMAKE_CURRENT_LIST_DIR}/misc/neopixel.pio)

target_link_libraries(ewand_core PRIVATE
        pico_stdlib
        hardware_pio
        hardware_pwm
        hardware_dma
        hardware_i2c
        pico_sync
        pico_multicore
        pico_rand
        pico_float
        pico_time
)

target_include_directories(ewand_core PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(ewand_core PRIVATE
        XOSC_KHZ=12000
        PLL_COMMON_REFDIV=1

        SYS_CLK_KHZ=240000
        PLL_SYS_VCO_FREQ_KHZ=1440000
        PLL_SYS_POSTDIV1=6
        PLL_SYS_POSTDIV2=1

        PICO_DEFAULT_UART_TX_PIN=16
        PICO_DEFAULT_UART_RX_PIN=17
)
pico_enable_stdio_usb(ewand_core 0)
pico_enable_stdio_uart(ewand_core 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(ewand_core)