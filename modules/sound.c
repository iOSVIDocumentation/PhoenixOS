#include "sound.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#ifndef PIN_BUZZER
#define PIN_BUZZER 21 
#endif

static bool snd_initialized = false;
static uint slice_num;

void snd_init(void) {
    if (snd_initialized) return;
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(PIN_BUZZER);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 1.0f);
    pwm_init(slice_num, &cfg, false);
    snd_initialized = true;
}

void snd_beep(uint16_t freq_hz, uint32_t duration_ms) {
    if (!snd_initialized || freq_hz == 0) {
        pwm_set_enabled(slice_num, false);
        return;
    }
    uint32_t clock_hz = 125000000; 
    uint32_t top = (clock_hz / freq_hz) - 1;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 1.0f);
    pwm_config_set_wrap(&cfg, top);
    pwm_init(slice_num, &cfg, true);
    pwm_set_gpio_level(PIN_BUZZER, top / 2);
    
    sleep_ms(duration_ms);
    
    pwm_set_enabled(slice_num, false);
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_put(PIN_BUZZER, 0);
}

void snd_click(void) { snd_beep(800, 30); }
void snd_error(void) { snd_beep(150, 200); }
void snd_shoot(void) { 
    snd_beep(200, 50); 
    sleep_ms(10);
    snd_beep(100, 50); 
}
void snd_success(void) { 
    snd_beep(600, 50); 
    sleep_ms(50);
    snd_beep(900, 100); 
}
