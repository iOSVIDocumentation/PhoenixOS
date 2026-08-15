/* Шаблон приложения PhoenixOS.
 * Скопируй в apps/app_myapp.c, переименуй app_myapp, добавь файл в CMakeLists.txt.
 * В main.c зарегистрируй ОДНОЙ строкой:
 *     extern const phoenix_app_t app_myapp;
 *     core_register_dyn(&app_myapp);
 * Открыть из любого места: core_open(core_find("myapp"), 0);
 * Файл НЕ подключён в сборку — это шаблон. */
#include "phoenix_core.h"
#include "st7789.h"
#include "buzzer.h"

static void my_enter(int arg) {
    (void)arg;
    st7789_fill(COLOR_WIN_BG);
    st7789_draw_string(8, 8, "My App", COLOR_WHITE, COLOR_WIN_BG, 2);
    st7789_draw_string(8, 40, "OK - beep, BACK - exit", COLOR_LIGHT_BLUE, COLOR_WIN_BG, 1);
}

static void my_tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;
    if (in->ok_pressed) buzzer_success();
    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }
}

static void my_exit(void) {}

const phoenix_app_t app_myapp = {
    .name = "myapp",
    .on_enter = my_enter,
    .on_tick = my_tick,
    .on_exit = my_exit,
};
