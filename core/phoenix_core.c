#include "phoenix_core.h"
#include "buttons.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include "fs_mutex.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

settings_t g_settings;

static const phoenix_app_t *apps[CORE_MAX_APPS];
static app_id_t cur = APP_DESKTOP;
static app_id_t pending = APP_INVALID;
static int pending_arg = 0;

static bool last_sw = false;
static uint32_t nav_time = 0;
static uint32_t last_tick_ms = 0;

#define WATCHDOG_TIMEOUT_MS 1500

void core_log(const char *msg) {
    FIL f;
    if (f_open(&f, "/reset.log", FA_WRITE | FA_OPEN_APPEND) != FR_OK) return;
    char line[48];
    int n = snprintf(line, sizeof(line), "t=%lu %s\n",
                     (unsigned long)to_ms_since_boot(get_absolute_time()), msg);
    UINT bw = 0;
    f_write(&f, line, n, &bw);
    f_close(&f);
}

bool core_set_cpu_mhz(uint16_t mhz) {
    if (!set_sys_clock_khz((uint32_t)mhz * 1000, true)) return false;
    spi_set_baudrate(spi0, 10 * 1000 * 1000);
    spi_set_baudrate(spi1, 12500000);
    return true;
}

static uint32_t thermal_time = 0;
static uint32_t last_hb = 0;
static uint8_t hb_bad = 0;
static void thermal_guard(void) {
    uint32_t hb = g_core1_heartbeat;
    if (hb == last_hb) {
        if (++hb_bad >= 3) watchdog_reboot(0, 0, 0);
    } else {
        hb_bad = 0;
    }
    last_hb = hb;

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
    if ((int)id < 0 || (int)id >= CORE_MAX_APPS || app == NULL) return;
    apps[id] = app;
}

app_id_t core_register_dyn(const phoenix_app_t *app) {
    if (app == NULL) return APP_INVALID;
    for (int i = (int)APP_COUNT; i < CORE_MAX_APPS; i++) {
        if (apps[i] == NULL) {
            apps[i] = app;
            return (app_id_t)i;
        }
    }
    return APP_INVALID;
}

app_id_t core_find(const char *name) {
    if (name == NULL) return APP_INVALID;
    for (int i = 0; i < CORE_MAX_APPS; i++) {
        if (apps[i] && apps[i]->name && strcmp(apps[i]->name, name) == 0) {
            return (app_id_t)i;
        }
    }
    return APP_INVALID;
}

void core_open(app_id_t id, int arg) {
    if ((int)id < 0 || (int)id >= CORE_MAX_APPS || apps[id] == NULL) return;
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
    fs_mutex_init();

    watchdog_enable(WATCHDOG_TIMEOUT_MS, true);

    if ((int)initial < 0 || (int)initial >= CORE_MAX_APPS || apps[initial] == NULL) {
        watchdog_reboot(0, 0, 0);
    }

    cur = initial;
    pending = APP_INVALID;
    apps[cur]->on_enter(0);

    last_tick_ms = to_ms_since_boot(get_absolute_time());

    while (true) {
        watchdog_update();

        core_input_t in;
        core_poll_input(&in);
        thermal_guard();

        bool consumed = false;
        if (cur != APP_MENU && in.start_pressed) {
            pending = APP_MENU;
            pending_arg = 0;
            consumed = true;
        }

        if (pending != APP_INVALID && apps[pending] != NULL) {
            if (apps[cur]->on_exit) apps[cur]->on_exit();
            cur = pending;
            pending = APP_INVALID;
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
        } else {
            pending = APP_INVALID;
        }

        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        uint32_t delta_ms = (last_tick_ms == 0) ? 16 : (now_ms - last_tick_ms);
        if (delta_ms > 100) delta_ms = 100;
        last_tick_ms = now_ms;

        apps[cur]->on_tick(&in, delta_ms);
        sleep_ms(10);
    }
}
