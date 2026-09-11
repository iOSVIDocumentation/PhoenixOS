#include "joystick.h"

void joystick_init(void) {
    // Инициализация ADC
    adc_init();
    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);

    // Инициализация кнопки (внутренняя подтяжка PULL_UP)
    gpio_init(JOY_SW_PIN);
    gpio_set_dir(JOY_SW_PIN, GPIO_IN);
    gpio_pull_up(JOY_SW_PIN);
}

static bool last_sw_raw = false;
static uint32_t last_sw_time = 0;

void joystick_read(joystick_state_t *state) {
    // Чтение оси X (ADC0)
    adc_select_input(0);
    state->x = adc_read(); // 0 - 4095

    // Чтение оси Y (ADC1)
    adc_select_input(1);
    state->y = adc_read(); // 0 - 4095

    // Чтение кнопки с debounce 20мс
    bool current_raw = (gpio_get(JOY_SW_PIN) == 0);
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (current_raw != last_sw_raw && (now - last_sw_time) > 20) {
        last_sw_raw = current_raw;
        last_sw_time = now;
    }
    state->sw_pressed = last_sw_raw;
}
