#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

#define SETTINGS_PATH "/phoenix.cfg"

typedef struct {
    uint8_t brightness;
    uint8_t cursor_speed;
    bool sound_enabled;
    uint16_t cpu_mhz;
    char wallpaper[64];
} settings_t;

void settings_load(settings_t *s);
void settings_save(const settings_t *s);
void settings_apply(const settings_t *s);

#endif // SETTINGS_H
