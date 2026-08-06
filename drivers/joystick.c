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

void joystick_read(joystick_state_t *state) {
    // Чтение оси X (ADC0)
    adc_select_input(0);
    state->x = adc_read(); // 0 - 4095

    // Чтение оси Y (ADC1)
    adc_select_input(1);
    state->y = adc_read(); // 0 - 4095

    // Чтение кнопки (0 = нажата, 1 = отпущена из-за PULL_UP)
    state->sw_pressed = (gpio_get(JOY_SW_PIN) == 0);
}
