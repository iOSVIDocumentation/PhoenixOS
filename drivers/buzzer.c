#include "buzzer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

static bool sound_on = true;

void buzzer_set_enabled(bool en) { sound_on = en; }
bool buzzer_is_enabled(void) { return sound_on; }

void buzzer_init(void) {
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_put(PIN_BUZZER, 0);
}

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms) {
    if (!sound_on) return;
    if (freq_hz == 0) {
        sleep_ms(duration_ms);
        return;
    }
    uint32_t period_us = 1000000 / freq_hz;
    uint32_t half_period_us = period_us / 2;
    uint32_t total_cycles = (freq_hz * duration_ms) / 1000;
    for (uint32_t i = 0; i < total_cycles; i++) {
        gpio_put(PIN_BUZZER, 1);
        busy_wait_us(half_period_us);
        gpio_put(PIN_BUZZER, 0);
        busy_wait_us(half_period_us);
    }
}

void buzzer_click(void) { buzzer_beep(2000, 30); }
void buzzer_error(void) { buzzer_beep(300, 300); }
void buzzer_success(void) { buzzer_beep(1000, 150); }

void buzzer_startup(void) {
    buzzer_beep(1000, 100);
    sleep_ms(50);
    buzzer_beep(1500, 100);
    sleep_ms(50);
    buzzer_beep(2000, 150);
}

void buzzer_tone_on(uint16_t freq_hz) {
    if (!sound_on || freq_hz == 0) {
        buzzer_tone_off();
        return;
    }
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    uint32_t clk = clock_get_hz(clk_sys);
    uint32_t div = 1;
    uint32_t wrap = clk / freq_hz;
    while (wrap > 65535 && div < 256) {
        div <<= 1;
        wrap = clk / ((uint32_t)freq_hz * div);
    }
    if (wrap == 0) wrap = 1;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, wrap - 1);
    pwm_config_set_clkdiv(&cfg, (float)div);
    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(PIN_BUZZER, wrap / 2);
}

void buzzer_tone_off(void) {
    uint slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    pwm_set_enabled(slice, false);
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_NULL);
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_put(PIN_BUZZER, 0);
}
