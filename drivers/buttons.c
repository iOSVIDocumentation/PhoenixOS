#include "buttons.h"

static const uint btn_pins[BTN_COUNT] = {BTN_START, BTN_OK, BTN_BACK};

static bool last_state[BTN_COUNT] = {true, true, true};
static uint32_t last_time[BTN_COUNT] = {0, 0, 0};

static int gpio_to_index(uint gpio) {
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btn_pins[i] == gpio) return i;
    }
    return -1;
}

void buttons_init(void) {
    for (int i = 0; i < BTN_COUNT; i++) {
        gpio_init(btn_pins[i]);
        gpio_set_dir(btn_pins[i], GPIO_IN);
        gpio_pull_up(btn_pins[i]);
    }
}

bool button_is_pressed(uint gpio) {
    return !gpio_get(gpio);
}

bool button_was_pressed(uint gpio) {
    int idx = gpio_to_index(gpio);
    if (idx < 0) return false;

    bool current = button_is_pressed(gpio);
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (current != last_state[idx] && (now - last_time[idx]) > 50) {
        last_state[idx] = current;
        last_time[idx] = now;
        return current;
    }
    return false;
}
