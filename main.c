#include <stdio.h>
#include "pico/stdlib.h"
#include "st7789.h"
#include "ff.h"
#include "hw_config.h"
#include "buzzer.h"
#include "joystick.h"
#include "buttons.h"
#include "settings.h"
#include "ui.h"
#include "phoenix_core.h"
#include "sysinfo.h"
#include "hardware/clocks.h"

extern const phoenix_app_t app_snake;

static bool safe_mode = false;

static void draw_bootscreen(void) {
    st7789_fill(COLOR_WIN_BG);
    st7789_draw_rect(10, 10, 300, 220, COLOR_WHITE);
    st7789_draw_rect(12, 12, 296, 216, COLOR_LIGHT_BLUE);
    st7789_draw_string(80, 45, "PhoenixOS", COLOR_WHITE, COLOR_WIN_BG, 3);
    st7789_draw_string(72, 80, "Retro Workstation v0.9.3", COLOR_LIGHT_BLUE, COLOR_WIN_BG, 1);
    st7789_draw_string(104, 95, "Kernel Edition", COLOR_LIGHT_BLUE, COLOR_WIN_BG, 1);
    st7789_draw_string(68, 110, "RP2350 (ARM Cortex-M33)", COLOR_DARK_GRAY, COLOR_WIN_BG, 1);
    if (safe_mode) {
        st7789_draw_string(60, 140, "SAFE MODE: settings reset", COLOR_RED, COLOR_WIN_BG, 1);
    }
    st7789_draw_string(45, 185, "Mounting SD (FAT32)...", COLOR_WHITE, COLOR_WIN_BG, 1);
}

int main(void) {
    stdio_init_all();

    st7789_init();
    buzzer_init();
    joystick_init();
    buttons_init();
    media_core1_init();
    safe_mode = button_is_pressed(BTN_BACK);

    buzzer_startup();
    draw_bootscreen();

    sd_card_t *pSD = sd_get_by_num(0);
    static FATFS fs;
    FRESULT fr = f_mount(&fs, pSD->pcName, 1);
    if (fr == FR_OK) {
        st7789_draw_string(45, 185, "SD Card Mounted! OK      ", COLOR_GREEN, COLOR_WIN_BG, 1);
        buzzer_success();
    } else {
        st7789_draw_string(45, 185, "SD Card Mount Failed!    ", COLOR_RED, COLOR_WIN_BG, 1);
        buzzer_error();
    }

    settings_load(&g_settings);
    if (safe_mode) {
        g_settings.cpu_mhz = 150;
        settings_save(&g_settings);
    }
    settings_apply(&g_settings);
    ui_set_wallpaper(g_settings.wallpaper);

    if (!safe_mode && g_settings.cpu_mhz != 150) {
        if (!core_set_cpu_mhz(g_settings.cpu_mhz)) {
            g_settings.cpu_mhz = 150;
        }
    }
    printf("[PhoenixOS] cpu: target=%u MHz, actual=%u MHz\n",
           (unsigned)g_settings.cpu_mhz,
           (unsigned)(clock_get_hz(clk_sys) / 1000000));

    sleep_ms(1500);

    core_register(APP_DESKTOP,   &app_desktop);
    core_register(APP_MENU,      &app_menu);
    core_register(APP_ABOUT,     &app_about);
    core_register(APP_FILES,     &app_files);
    core_register(APP_VIEWER,    &app_viewer);
    core_register(APP_SETTINGS,  &app_settings);
    core_register(APP_WALLPAPER, &app_wallpaper);
    core_register(APP_CPU,       &app_cpu);
    core_register(APP_MEDIA,     &app_media);
    core_register(APP_WOLF3D,    &app_wolf3d);
    core_register(APP_SNAKE, &app_snake);

    core_start(APP_DESKTOP);
    return 0;
}
