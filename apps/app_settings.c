#include "phoenix_core.h"

#define SETTINGS_NAV_DELAY_MS 200
static uint32_t settings_last_nav = 0;
#include "ui.h"
#include "settings.h"
#include "theme.h"
#include "theme_icons.h"
#include "wallpaper.h"
#include "buzzer.h"
#include "st7789.h"
#include <stdio.h>

static void set_change(settings_t *s, int row, int dir) {
    switch (row) {
        case 0: {
            const uint8_t lv[4] = {25, 50, 75, 100};
            int i;
            for (i = 0; i < 4; i++) if (lv[i] == s->brightness) break;
            if (i == 4) i = 3;
            i = (i + dir + 4) % 4;
            s->brightness = lv[i];
            st7789_set_backlight(s->brightness);
            break;
        }
        case 1: {
            int v = s->cursor_speed + dir;
            if (v < 1) v = 3;
            if (v > 3) v = 1;
            s->cursor_speed = (uint8_t)v;
            break;
        }
        case 2:
            s->sound_enabled = !s->sound_enabled;
            buzzer_set_enabled(s->sound_enabled);
            break;
        case 3: {
            int t = (s->theme + dir + THEME_COUNT) % THEME_COUNT;
            s->theme = (uint8_t)t;
            theme_set(t);
            if (!theme_icons_load(t)) {
                buzzer_error();
            }
            break;
        }
        default:
            break;
    }
}

static int set_sel = 0;

static void settings_enter(int arg) {
    if (arg == 0) set_sel = 0;
    ui_draw_settings_window(set_sel, &g_settings);
}

static void settings_tick(const core_input_t *in, uint32_t delta_ms) {
    int old_sel = set_sel;

    uint32_t now = core_now_ms();
    bool nav_ok = (now - settings_last_nav >= SETTINGS_NAV_DELAY_MS);

    if (in->nav_up && set_sel > 0 && nav_ok) {
        set_sel--;
        settings_last_nav = now;
        buzzer_click();
        ui_settings_row(old_sel, set_sel, &g_settings);
        ui_settings_row(set_sel, set_sel, &g_settings);
    } else if (in->nav_down && set_sel < SET_ROWS - 1 && nav_ok) {
        set_sel++;
        settings_last_nav = now;
        buzzer_click();
        ui_settings_row(old_sel, set_sel, &g_settings);
        ui_settings_row(set_sel, set_sel, &g_settings);
    } else if (in->nav_left && set_sel < 4 && nav_ok) {
        set_change(&g_settings, set_sel, -1);
        settings_last_nav = now;
        buzzer_click();
        if (set_sel == 3) ui_draw_settings_window(set_sel, &g_settings);
        else ui_settings_row(set_sel, set_sel, &g_settings);
    } else if (in->nav_right && set_sel < 4 && nav_ok) {
        set_change(&g_settings, set_sel, +1);
        settings_last_nav = now;
        buzzer_click();
        if (set_sel == 3) ui_draw_settings_window(set_sel, &g_settings);
        else ui_settings_row(set_sel, set_sel, &g_settings);
    }

    if (in->ok_pressed) {
        if (set_sel == 4) {
            buzzer_click();
            core_open(APP_WALLPAPER, 0);
        } else if (set_sel == 5) {
            buzzer_click();
            core_open(APP_CPU, 0);
        } else {
            set_change(&g_settings, set_sel, +1);
            buzzer_click();
            if (set_sel == 3) ui_draw_settings_window(set_sel, &g_settings);
            else ui_settings_row(set_sel, set_sel, &g_settings);
        }
    }

    if (in->back_pressed) {
        settings_save(&g_settings);
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }
}

const phoenix_app_t app_settings = {
    .name = "settings",
    .on_enter = settings_enter,
    .on_tick = settings_tick,
};

static int wp_sel = 0;
static int wp_scroll = 0;

static void wp_enter(int arg) {
    (void)arg;
    wp_sel = 0;
    wp_scroll = 0;
    wallpaper_scan();
    ui_draw_wallpaper_picker(wp_sel, wp_scroll, g_settings.wallpaper);
}

static void wp_tick(const core_input_t *in, uint32_t delta_ms) {
    int old_sel = wp_sel;
    int old_scroll = wp_scroll;
    bool applied = false;
    int total = wallpaper_count() + 1;

    uint32_t now_wp = core_now_ms();
    bool nav_ok_wp = (now_wp - settings_last_nav >= SETTINGS_NAV_DELAY_MS);

    if (in->nav_up && wp_sel > 0 && nav_ok_wp) {
        wp_sel--;
        settings_last_nav = now_wp;
        buzzer_click();
    } else if (in->nav_down && wp_sel < total - 1 && nav_ok_wp) {
        wp_sel++;
        settings_last_nav = now_wp;
        buzzer_click();
    }

    if (wp_sel < wp_scroll) wp_scroll = wp_sel;
    if (wp_sel >= wp_scroll + WP_VISIBLE_ROWS) wp_scroll = wp_sel - WP_VISIBLE_ROWS + 1;

    if (in->ok_pressed) {
        buzzer_click();
        if (wp_sel == 0) {
            g_settings.wallpaper[0] = 0;
            ui_set_wallpaper("");
        } else {
            snprintf(g_settings.wallpaper, sizeof(g_settings.wallpaper),
                     "/wallpapers/%s", wallpaper_name(wp_sel - 1));
            ui_set_wallpaper(g_settings.wallpaper);
        }
        applied = true;
    }

    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_SETTINGS, 1);
        return;
    }

    if (wp_scroll != old_scroll) {
        ui_draw_wallpaper_picker(wp_sel, wp_scroll, g_settings.wallpaper);
    } else if (wp_sel != old_sel) {
        ui_picker_row(old_sel, wp_sel, g_settings.wallpaper, 20 + (old_sel - wp_scroll) * 18);
        ui_picker_row(wp_sel, wp_sel, g_settings.wallpaper, 20 + (wp_sel - wp_scroll) * 18);
    } else if (applied) {
        ui_picker_row(wp_sel, wp_sel, g_settings.wallpaper, 20 + (wp_sel - wp_scroll) * 18);
    }
}

const phoenix_app_t app_wallpaper = {
    .name = "wallpaper",
    .on_enter = wp_enter,
    .on_tick = wp_tick,
};

static const uint16_t cpu_opts[4] = {150, 200, 225, 250};
static int cpu_sel = 4;

static void cpu_enter(int arg) {
    (void)arg;
    cpu_sel = 4;
    ui_draw_cpu_menu(cpu_sel, g_settings.cpu_mhz);
}

static void cpu_tick(const core_input_t *in, uint32_t delta_ms) {
    int old_sel = cpu_sel;
    bool applied = false;

    uint32_t now_cpu = core_now_ms();
    bool nav_ok_cpu = (now_cpu - settings_last_nav >= SETTINGS_NAV_DELAY_MS);

    if (in->nav_up && cpu_sel > 0 && nav_ok_cpu) {
        cpu_sel--;
        settings_last_nav = now_cpu;
        buzzer_click();
    } else if (in->nav_down && cpu_sel < 4 && nav_ok_cpu) {
        cpu_sel++;
        settings_last_nav = now_cpu;
        buzzer_click();
    }

    if (in->ok_pressed) {
        if (cpu_sel == 4) {
            buzzer_click();
            core_open(APP_SETTINGS, 1);
            return;
        } else {
            uint16_t v = cpu_opts[cpu_sel];
            if (v == g_settings.cpu_mhz) {
                buzzer_click();
            } else if (core_set_cpu_mhz(v)) {
                g_settings.cpu_mhz = v;
                settings_save(&g_settings);
                buzzer_success();
            } else {
                buzzer_error();
            }
            applied = true;
        }
    }

    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_SETTINGS, 1);
        return;
    }

    if (cpu_sel != old_sel) {
        ui_cpu_row(old_sel, cpu_sel, g_settings.cpu_mhz);
        ui_cpu_row(cpu_sel, cpu_sel, g_settings.cpu_mhz);
    } else if (applied) {
        ui_cpu_row(cpu_sel, cpu_sel, g_settings.cpu_mhz);
    }
}

const phoenix_app_t app_cpu = {
    .name = "cpu",
    .on_enter = cpu_enter,
    .on_tick = cpu_tick,
};
