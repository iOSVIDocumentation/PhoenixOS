#include <stdio.h>
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "st7789.h"
#include "ff.h"
#include "hw_config.h"
#include "buzzer.h"
#include "joystick.h"
#include "buttons.h"
#include "settings.h"
#include "theme.h"
#include "theme_icons.h"
#include "provision.h"
#include "ui.h"
#include "phoenix_core.h"
#include "hardware/watchdog.h"
#include "sysinfo.h"

extern const phoenix_app_t app_snake;
extern const phoenix_app_t app_games;
extern const phoenix_app_t app_tetris;
extern app_id_t g_games_snake_id;
extern app_id_t g_games_tetris_id;
extern app_id_t g_games_cube_id;
extern const phoenix_app_t app_cube;
extern const phoenix_app_t app_calc;
app_id_t g_apps_calc_id;

static bool safe_mode = false;

static const char *reset_reason_str(void) {
    if (watchdog_enable_caused_reboot()) return "LAST: WDT TIMEOUT";
    if (watchdog_caused_reboot())        return "LAST: WDT REBOOT";
    return "LAST: POWER ON";
}

int main(void) {
    stdio_init_all();

    st7789_init();
    buzzer_init();
    adc_init();
    joystick_init();
    buttons_init();
    media_core1_init();
    safe_mode = button_is_pressed(BTN_BACK);

    buzzer_startup();
    ui_draw_bootscreen(safe_mode, -1);
    ui_bootscreen_status(reset_reason_str(), BOOT_GREEN);

    sd_card_t *pSD = sd_get_by_num(0);
    static FATFS fs;
    FRESULT fr = f_mount(&fs, pSD->pcName, 1);
    if (fr == FR_OK) {
        provision_sd_card();
        theme_icons_provision();

        /* журнал причин перезагрузки */
        FIL lf;
        if (f_open(&lf, "/reset.log", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
            char line[64];
            int n = snprintf(line, sizeof(line), "t=%lu %s\n",
                             (unsigned long)to_ms_since_boot(get_absolute_time()),
                             watchdog_enable_caused_reboot() ? "WDT_TIMEOUT" :
                             watchdog_caused_reboot() ? "WDT_REBOOT" : "POWER_ON");
            UINT bw = 0;
            f_write(&lf, line, n, &bw);
            f_close(&lf);
        }
        ui_bootscreen_status("Scanning SD free space...", BOOT_GREEN);
        sysinfo_sd_scan();
        buzzer_success();
    } else {
        buzzer_error();
    }

    settings_load(&g_settings);
    if (safe_mode) {
        g_settings.cpu_mhz = 150;
        g_settings.theme = THEME_PHOENIX;
        settings_save(&g_settings);
    }
    settings_apply(&g_settings);
    theme_icons_load(g_settings.theme);
    ui_set_wallpaper(g_settings.wallpaper);
    ui_draw_bootscreen(safe_mode, (fr == FR_OK) ? 1 : 0);
    ui_bootscreen_status(reset_reason_str(), BOOT_GREEN);

    if (!safe_mode && g_settings.cpu_mhz != 150) {
        if (!core_set_cpu_mhz(g_settings.cpu_mhz)) {
            g_settings.cpu_mhz = 150;
        }
    }

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

    core_register(APP_SNAKE,     &app_games);
    g_games_snake_id  = core_register_dyn(&app_snake);
    g_games_tetris_id = core_register_dyn(&app_tetris);
    g_games_cube_id = core_register_dyn(&app_cube);
    g_apps_calc_id = core_register_dyn(&app_calc);

    core_start(APP_DESKTOP);
    return 0;
}
