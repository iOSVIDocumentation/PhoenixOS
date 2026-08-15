#include "sound_svc.h"
#include "board.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define SND_Q_SIZE 16

typedef struct { uint16_t freq; uint16_t dur; } snd_req_t;

static snd_req_t q[SND_Q_SIZE];
static volatile int q_head = 0, q_tail = 0;
static mutex_t q_mutex;
static volatile bool enabled = true;
static bool initialized = false;

/* ---- состояние движка (только core1) ---- */
static bool pin_init = false;
static bool playing = false;
static uint16_t cur_freq = 0;
static uint32_t tone_end = 0; /* 0 = постоянный тон */

void sound_svc_init(void) {
    if (initialized) return;
    mutex_init(&q_mutex);
    initialized = true;
}

void sound_svc_set_enabled(bool en) { enabled = en; }

void sound_svc_request(uint16_t f, uint16_t d) {
    if (!initialized) sound_svc_init();
    mutex_enter_blocking(&q_mutex);
    int next = (q_head + 1) % SND_Q_SIZE;
    if (next != q_tail) {
        q[q_head].freq = f;
        q[q_head].dur = d;
        q_head = next;
    }
    mutex_exit(&q_mutex);
}

void sound_svc_tone_on(uint16_t f)  { sound_svc_request(f, 0); }
void sound_svc_tone_off(void)       { sound_svc_request(0, 0); }

/* ---- HW: PWM буззера, владеет ТОЛЬКО core1 ---- */
static void hw_off(void) {
    uint slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    pwm_set_enabled(slice, false);
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_NULL);
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_put(PIN_BUZZER, 0);
}

static void hw_on(uint16_t freq) {
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_PWM);
    uint32_t clk = clock_get_hz(clk_sys);
    uint32_t div = 1;
    uint32_t wrap = clk / freq;
    while (wrap > 65535 && div < 256) {
        div <<= 1;
        wrap = clk / ((uint32_t)freq * div);
    }
    if (wrap == 0) wrap = 1;
    uint slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, wrap - 1);
    pwm_config_set_clkdiv(&cfg, (float)div);
    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(PIN_BUZZER, wrap / 2);
}

static bool q_pop(snd_req_t *r) {
    if (q_tail == q_head) return false;
    *r = q[q_tail];
    q_tail = (q_tail + 1) % SND_Q_SIZE;
    return true;
}

void sound_svc_core1_tick(bool movie_active) {
    if (!pin_init) { hw_off(); pin_init = true; }

    uint32_t now = to_ms_since_boot(get_absolute_time());

    /* закончился timed-тон */
    if (playing && tone_end != 0 && (int32_t)(now - tone_end) >= 0) {
        hw_off();
        playing = false;
    }

    /* видео играет — UI-звуки глушим (приоритет) */
    if (movie_active) {
        mutex_enter_blocking(&q_mutex);
        q_head = q_tail = 0;
        mutex_exit(&q_mutex);
        return;
    }

    if (!playing) {
        snd_req_t r;
        mutex_enter_blocking(&q_mutex);
        bool has = q_pop(&r);
        mutex_exit(&q_mutex);
        if (has) {
            if (enabled && r.freq) hw_on(r.freq);
            cur_freq = r.freq;
            if (r.freq == 0) {
                playing = false;              /* стоп/пауза: очередь идёт дальше */
            } else {
                playing = true;
                tone_end = (r.dur == 0) ? 0 : now + r.dur;
            }
        }
    }
}

void sound_svc_movie_note(uint16_t f, uint16_t d) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (enabled && f) hw_on(f); else hw_off();
    cur_freq = f;
    playing = true;
    tone_end = now + d;
}
