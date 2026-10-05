#include "phoenix_core.h"
#include "buttons.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include "fs_mutex.h"
#include "logger.h"
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
    LOG_INFO(LOG_SUB_BOOT, "%s", msg ? msg : "(null)");
}

bool core_set_cpu_mhz(uint16_t mhz) {
    LOG_DEBUG(LOG_SUB_CORE, "cpu_set requested=%uMHz", mhz);
    if (mhz < 150 || mhz > 300) {
        LOG_ERROR(LOG_SUB_CORE, "cpu_set invalid=%uMHz", mhz);
        return false;
    }
    if (!set_sys_clock_khz((uint32_t)mhz * 1000, true)) {
        LOG_ERROR(LOG_SUB_CORE, "cpu_set pll_failed=%uMHz", mhz);
        return false;
    }
    spi_set_baudrate(spi0, 55 * 1000 * 1000);
    spi_set_baudrate(spi1, 12500000);
    LOG_INFO(LOG_SUB_CORE, "cpu_set ok=%uMHz spi0=55MHz spi1=12.5MHz", mhz);
    return true;
}

static uint32_t thermal_time = 0;
static uint32_t last_hb = 0;
static uint8_t hb_bad = 0;
static void thermal_guard(void) {
    uint32_t hb = g_core1_heartbeat;
    if (hb == last_hb) {
        if (++hb_bad >= 3) {
            LOG_ERROR(LOG_SUB_WDT, "core1_heartbeat_timeout hb=%lu bad=%u",
                      (unsigned long)hb, hb_bad);
            logger_flush_now();
            watchdog_reboot(0, 0, 0);
        } else if (hb_bad == 1) {
            LOG_WARN(LOG_SUB_WDT, "core1_heartbeat_stalled hb=%lu",
                     (unsigned long)hb);
        }
    } else {
        if (hb_bad) {
            LOG_DEBUG(LOG_SUB_WDT, "core1_heartbeat_recovered hb=%lu",
                      (unsigned long)hb);
        }
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
    int32_t temp_limit = (g_settings.cpu_mhz >= 300) ? 60 : 65;

    static uint32_t last_temp_log = 0;
    if (t >= temp_limit - 5 || now - last_temp_log >= 10000) {
        LOG_DEBUG(LOG_SUB_THERMAL,
                  "temp=%dC limit=%dC cpu=%uMHz raw=%lu mv=%ld",
                  (int)t, (int)temp_limit, g_settings.cpu_mhz,
                  (unsigned long)raw, (long)mv);
        last_temp_log = now;
    }

    if (t > temp_limit) {
        LOG_ERROR(LOG_SUB_THERMAL,
                  "thermal_rollback temp=%dC limit=%dC cpu=%uMHz -> 150MHz",
                  (int)t, (int)temp_limit, g_settings.cpu_mhz);
        g_settings.cpu_mhz = 150;
        settings_save(&g_settings);
        logger_flush_now();
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
    if ((int)id < 0 || (int)id >= CORE_MAX_APPS || apps[id] == NULL) {
        LOG_WARN(LOG_SUB_APP, "open_invalid id=%d arg=%d", (int)id, arg);
        return;
    }
    LOG_DEBUG(LOG_SUB_APP, "open id=%d name=%s arg=%d",
              (int)id, apps[id]->name ? apps[id]->name : "?", arg);
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
    adc_init();
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
        logger_tick();

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
            const char *from_name = (apps[cur] && apps[cur]->name) ? apps[cur]->name : "?";
            const char *to_name = (apps[pending] && apps[pending]->name) ? apps[pending]->name : "?";
            int arg = pending_arg;
            LOG_DEBUG(LOG_SUB_APP, "switch from=%s to=%s arg=%d",
                      from_name, to_name, arg);
            if (apps[cur]->on_exit) apps[cur]->on_exit();
            cur = pending;
            pending = APP_INVALID;
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
        sleep_until(last_tick_ms * 1000);
    }
}
