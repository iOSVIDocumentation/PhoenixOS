#include "phoenix_core.h"
#include "ui.h"
#include "buzzer.h"

static int cursor_x = 160;
static int cursor_y = 100;
static int last_hover = -1;

static void desktop_enter(int arg) {
    (void)arg;
    last_hover = -1;
    ui_draw_desktop();
    ui_draw_cursor(cursor_x, cursor_y);
}

static void desktop_tick(const core_input_t *in, uint32_t delta_ms) {
    int speed = g_settings.cursor_speed;

    int new_x = cursor_x + in->dx * speed;
    int new_y = cursor_y + in->dy * speed;
    if (new_x < 0) new_x = 0;
    if (new_x > 320 - UI_CURSOR_W) new_x = 320 - UI_CURSOR_W;
    if (new_y < 0) new_y = 0;
    if (new_y > 198) new_y = 198;

    int hover = ui_hit_test(new_x, new_y);

    if (new_x != cursor_x || new_y != cursor_y || hover != last_hover) {
        ui_invalidate_rect(cursor_x, cursor_y, UI_CURSOR_W, UI_CURSOR_H);
        ui_refresh_hover(hover);
        last_hover = hover;
        cursor_x = new_x;
        cursor_y = new_y;
        ui_draw_cursor(cursor_x, cursor_y);
    }

    if (in->ok_pressed || in->sw_pressed) {
        if (last_hover == ICON_MY_PC) {
            buzzer_click();
            core_open(APP_ABOUT, 0);
        } else if (last_hover == ICON_FILES) {
            buzzer_click();
            core_open(APP_FILES, 0);
        } else if (last_hover == ICON_SETTINGS) {
            buzzer_click();
            core_open(APP_SETTINGS, 0);
        } else if (last_hover == ICON_MEDIA) {
            buzzer_click();
            media_open_path("");
            core_open(APP_MEDIA, 0);
        } else if (last_hover != -1) {
            buzzer_click();
            core_open(APP_WOLF3D, 0);
        }
    }
}

const phoenix_app_t app_desktop = {
    .name = "desktop",
    .on_enter = desktop_enter,
    .on_tick = desktop_tick,
};
