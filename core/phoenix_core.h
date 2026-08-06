#ifndef PHOENIX_CORE_H
#define PHOENIX_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "joystick.h"
#include "settings.h"

typedef enum {
    APP_DESKTOP,
    APP_MENU,
    APP_ABOUT,
    APP_FILES,
    APP_VIEWER,
    APP_SETTINGS,
    APP_WALLPAPER,
    APP_CPU,
    APP_MEDIA,
    APP_WOLF3D,
    APP_COUNT
} app_id_t;

typedef struct {
    bool start_pressed, ok_pressed, back_pressed, sw_pressed;
    bool nav_up, nav_down, nav_left, nav_right;
    int dx, dy;
    joystick_state_t raw;
} core_input_t;

typedef struct {
    const char *name;
    void (*on_enter)(int arg);
    void (*on_tick)(const core_input_t *in, uint32_t delta_ms);
    void (*on_exit)(void);
} phoenix_app_t;

extern settings_t g_settings;

void core_register(app_id_t id, const phoenix_app_t *app);
void core_open(app_id_t id, int arg);
void core_start(app_id_t initial);

bool core_set_cpu_mhz(uint16_t mhz);

void media_open_path(const char *path);
void media_core1_init(void);
extern const phoenix_app_t app_desktop;
extern const phoenix_app_t app_menu;
extern const phoenix_app_t app_about;
extern const phoenix_app_t app_files;
extern const phoenix_app_t app_viewer;
extern const phoenix_app_t app_settings;
extern const phoenix_app_t app_wallpaper;
extern const phoenix_app_t app_cpu;
extern const phoenix_app_t app_media;
extern const phoenix_app_t app_wolf3d;

#endif // PHOENIX_CORE_H
