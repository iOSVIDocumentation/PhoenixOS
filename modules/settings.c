#include "settings.h"
#include "theme.h"
#include "ff.h"
#include "st7789.h"
#include "buzzer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void defaults(settings_t *s) {
    s->brightness = 100;
    s->cursor_speed = 2;
    s->sound_enabled = true;
    s->cpu_mhz = 150;
    s->theme = THEME_PHOENIX;
    s->wallpaper[0] = 0;
}

void settings_load(settings_t *s) {
    defaults(s);
    FIL f;
    if (f_open(&f, SETTINGS_PATH, FA_READ) != FR_OK) return;
    char buf[256];
    UINT br = 0;
    f_read(&f, buf, sizeof(buf) - 1, &br);
    f_close(&f);
    buf[br] = 0;

    char *p = buf;
    while (p && *p) {
        char *eol = strchr(p, '\n');
        if (eol) *eol = 0;
        char *cr = strchr(p, '\r');
        if (cr) *cr = 0;
        char *eq = strchr(p, '=');
        if (eq) {
            *eq = 0;
            const char *key = p;
            const char *val = eq + 1;
            if (strcmp(key, "bright") == 0) {
                int v = atoi(val);
                if (v == 25 || v == 50 || v == 75 || v == 100) s->brightness = (uint8_t)v;
            } else if (strcmp(key, "cursor") == 0) {
                int v = atoi(val);
                if (v >= 1 && v <= 3) s->cursor_speed = (uint8_t)v;
            } else if (strcmp(key, "sound") == 0) {
                s->sound_enabled = (strcmp(val, "on") == 0);
            } else if (strcmp(key, "cpu") == 0) {
                int v = atoi(val);
                if (v == 150 || v == 200 || v == 225 || v == 250) s->cpu_mhz = (uint16_t)v;
            } else if (strcmp(key, "theme") == 0) {
                int v = atoi(val);
                if (v >= 0 && v < THEME_COUNT) s->theme = (uint8_t)v;
            } else if (strcmp(key, "wallpaper") == 0) {
                strncpy(s->wallpaper, val, sizeof(s->wallpaper) - 1);
                s->wallpaper[sizeof(s->wallpaper) - 1] = 0;
            }
        }
        if (!eol) break;
        p = eol + 1;
    }
}

void settings_save(const settings_t *s) {
    FIL f;
    if (f_open(&f, SETTINGS_PATH, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    char buf[224];
    int n = snprintf(buf, sizeof(buf), "bright=%u\ncursor=%u\nsound=%s\ncpu=%u\ntheme=%u\nwallpaper=%s\n",
                     s->brightness, s->cursor_speed,
                     s->sound_enabled ? "on" : "off", s->cpu_mhz,
                     s->theme, s->wallpaper);
    UINT bw = 0;
    f_write(&f, buf, n, &bw);
    f_close(&f);
}

void settings_apply(const settings_t *s) {
    theme_set(s->theme);
    st7789_set_backlight(s->brightness);
    buzzer_set_enabled(s->sound_enabled);
}
