#include "settings.h"
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
    s->cpu_mhz = 250; /* разгон по умолчанию */
    s->wallpaper[0] = 0;
}

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void settings_load(settings_t *s) {
    defaults(s);
    FIL f;
    if (f_open(&f, SETTINGS_PATH, FA_READ) != FR_OK) {
        /* файла нет (пустая карта или первый запуск) — создаём с дефолтами */
        settings_save(s);
        return;
    }
    char buf[256];
    UINT br = 0;
    f_read(&f, buf, sizeof(buf) - 1, &br);
    f_close(&f);
    if (br == 0) {
        /* файл пустой — перезаписываем дефолтами */
        settings_save(s);
        return;
    }
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
                s->brightness = (uint8_t)clamp_int(atoi(val), 0, 100);
            } else if (strcmp(key, "cursor") == 0) {
                s->cursor_speed = (uint8_t)clamp_int(atoi(val), 1, 3);
            } else if (strcmp(key, "sound") == 0) {
                s->sound_enabled = (strcmp(val, "on") == 0);
            } else if (strcmp(key, "cpu") == 0) {
                s->cpu_mhz = (uint16_t)clamp_int(atoi(val), 100, 300);
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
    char buf[192];
    int n = snprintf(buf, sizeof(buf), "bright=%u\ncursor=%u\nsound=%s\ncpu=%u\nwallpaper=%s\n",
                     s->brightness, s->cursor_speed,
                     s->sound_enabled ? "on" : "off", s->cpu_mhz, s->wallpaper);
    UINT bw = 0;
    f_write(&f, buf, n, &bw);
    f_close(&f);
}

void settings_apply(const settings_t *s) {
    st7789_set_backlight(s->brightness);
    buzzer_set_enabled(s->sound_enabled);
}
