cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

project(PhoenixOS C CXX ASM)

pico_sdk_init()

add_subdirectory(fatfs_lib/FatFs_SPI fatfs_build)

add_executable(PhoenixOS
    main.c
    core/phoenix_core.c
    core/fs_mutex.c
    core/sound_svc.c
    core/provision.c
    apps/app_desktop.c
    apps/app_system.c
    apps/app_snake.c
    apps/app_games.c
    apps/app_tetris.c
    apps/app_cube.c
    apps/app_calc.c
    apps/app_files.c
    apps/app_viewer.c
    apps/app_settings.c
    apps/app_media.c
    drivers/st7789.c
    drivers/buzzer.c
    drivers/joystick.c
    drivers/buttons.c
    modules/ui.c
    modules/files.c
    modules/viewer.c
    modules/settings.c
    modules/wallpaper.c
    modules/sysinfo.c
    modules/theme.c
    modules/theme_icons.c
    modules/sound.c
    modules/hw_config.c
)

target_include_directories(PhoenixOS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/apps
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/modules
)

target_link_libraries(PhoenixOS
    pico_stdlib
    pico_multicore
    hardware_spi
    hardware_gpio
    hardware_pwm
    hardware_adc
    FatFs_SPI
)

pico_enable_stdio_usb(PhoenixOS 1)
pico_enable_stdio_uart(PhoenixOS 0)

pico_add_extra_outputs(PhoenixOS)
