#include "phoenix_core.h"
#include "buttons.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"
#include <string.h>

settings_t g_settings;

static const phoenix_app_t *apps[APP_COUNT];
static app_id_t cur = APP_DESKTOP;
static app_id_t pending = APP_COUNT;
static int pending_arg = 0;

static bool last_sw = false;
static uint32_t nav_time = 0;

bool core_set_cpu_mhz(uint16_t mhz) {
    if (!set_sys_clock_khz((uint32_t)mhz * 1000, true)) return false;
    spi_set_baudrate(spi0, 10 * 1000 * 1000);
    spi_set_baudrate(spi1, 12500000);
    return true;
}

static uint32_t thermal_time = 0;
static void thermal_guard(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - thermal_time < 1000) return;
    thermal_time = now;
    adc_select_input(4);
    uint32_t raw = adc_read();
    int32_t mv = (int32_t)(raw * 3300) / 4095;
    int32_t t = 27 - ((mv - 706) * 1000) / 1710;
    if (t > 65) {
        g_settings.cpu_mhz = 150;
        settings_save(&g_settings);
        watchdog_reboot(0, 0, 0);
    }
}

void core_register(app_id_t id, const phoenix_app_t *app) {
    apps[id] = app;
}

void core_open(app_id_t id, int arg) {
    pending = id;
    pending_arg = arg;
}

static void core_poll_input(core_input_t *in) {
    memset(in, 0, sizeof(*in));
    joystick_read(&in->raw);

    bool sw_edge = in->raw.sw_pressed && !last_sw;
    last_sw = in->raw.sw_pressed;
    in->sw_pressed = sw_edge;

    in->start_pressed = button_was_pressed(BTN_START);
    in->ok_pressed    = button_was_pressed(BTN_OK) || sw_edge;
    in->back_pressed  = button_was_pressed(BTN_BACK);

    in->dx = (in->raw.x < 1800) ? -1 : (in->raw.x > 2200) ? 1 : 0;
    in->dy = (in->raw.y < 1800) ? -1 : (in->raw.y > 2200) ? 1 : 0;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - nav_time > 150) {
        if (in->dy < 0)      { in->nav_up = true;    nav_time = now; }
        else if (in->dy > 0) { in->nav_down = true;  nav_time = now; }
        else if (in->dx < 0) { in->nav_left = true;  nav_time = now; }
        else if (in->dx > 0) { in->nav_right = true; nav_time = now; }
    }
}

void core_start(app_id_t initial) {
    adc_set_temp_sensor_enabled(true);
    cur = initial;
    pending = APP_COUNT;
    apps[cur]->on_enter(0);

    while (true) {
        core_input_t in;
        core_poll_input(&in);
        thermal_guard();

        bool consumed = false;
        if (cur != APP_MENU && in.start_pressed) {
            pending = APP_MENU;
            pending_arg = 0;
            consumed = true;
        }

        if (pending != APP_COUNT) {
            if (apps[cur]->on_exit) apps[cur]->on_exit();
            cur = pending;
            pending = APP_COUNT;
            int arg = pending_arg;
            pending_arg = 0;
            apps[cur]->on_enter(arg);
            if (consumed) {
                in.start_pressed = false;
                in.ok_pressed = false;
                in.back_pressed = false;
                in.sw_pressed = false;
                in.nav_up = in.nav_down = in.nav_left = in.nav_right = false;
            }
        }

        apps[cur]->on_tick(&in);
        sleep_ms(10);
    }
}
