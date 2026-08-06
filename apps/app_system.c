#include "phoenix_core.h"
#include "ui.h"
#include "buzzer.h"
#include "st7789.h"
#include "hardware/watchdog.h"

/* ---------- Меню "Пуск" ---------- */

static int menu_sel = 0;

static void menu_enter(int arg) {
    (void)arg;
    menu_sel = 0;
    ui_draw_start_menu(menu_sel);
}

static void menu_tick(const core_input_t *in) {
    if (in->nav_up) {
        menu_sel = (menu_sel + UI_MENU_ITEMS - 1) % UI_MENU_ITEMS;
        buzzer_click();
        ui_draw_start_menu(menu_sel);
    } else if (in->nav_down) {
        menu_sel = (menu_sel + 1) % UI_MENU_ITEMS;
        buzzer_click();
        ui_draw_start_menu(menu_sel);
    }

    if (in->start_pressed || in->back_pressed) {
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }

    if (in->ok_pressed) {
        buzzer_click();
        if (menu_sel == 0) {
            core_open(APP_ABOUT, 0);
        } else {
            st7789_set_backlight(0);
            watchdog_reboot(0, 0, 0);
        }
    }
}

const phoenix_app_t app_menu = {
    .name = "menu",
    .on_enter = menu_enter,
    .on_tick = menu_tick,
};

/* ---------- О системе ---------- */

static void about_enter(int arg) {
    (void)arg;
    ui_draw_about_window();
}

static void about_tick(const core_input_t *in) {
    if (in->ok_pressed || in->back_pressed) {
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }
}

const phoenix_app_t app_about = {
    .name = "about",
    .on_enter = about_enter,
    .on_tick = about_tick,
};
