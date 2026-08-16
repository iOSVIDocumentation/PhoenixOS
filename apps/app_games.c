#include "phoenix_core.h"
#include "ui.h"
#include "buzzer.h"

/* id ставит main.c после регистрации */
app_id_t g_games_snake_id  = APP_INVALID;
app_id_t g_games_tetris_id = APP_INVALID;

static int g_sel = 0;

static void games_enter(int arg) {
    (void)arg;
    g_sel = 0;
    ui_draw_games_menu(g_sel);
}

static void games_tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;
    int old = g_sel;

    if (in->nav_up && g_sel > 0) {
        g_sel--;
        buzzer_click();
        ui_games_row(old, g_sel, 20 + old * 20);
        ui_games_row(g_sel, g_sel, 20 + g_sel * 20);
    } else if (in->nav_down && g_sel < GAMES_ROWS - 1) {
        g_sel++;
        buzzer_click();
        ui_games_row(old, g_sel, 20 + old * 20);
        ui_games_row(g_sel, g_sel, 20 + g_sel * 20);
    }

    if (in->ok_pressed) {
        buzzer_click();
        if (g_sel == 0 && g_games_snake_id != APP_INVALID) {
            core_open(g_games_snake_id, 0);
        } else if (g_sel == 1 && g_games_tetris_id != APP_INVALID) {
            core_open(g_games_tetris_id, 0);
        }
    }

    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }
}

const phoenix_app_t app_games = {
    .name = "games",
    .on_enter = games_enter,
    .on_tick = games_tick,
};
