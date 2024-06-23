add_executable(ewand_core
    main.c
)

target_link_libraries(ewand_core pico_stdlib)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(ewand_core)