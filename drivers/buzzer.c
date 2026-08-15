#include "buzzer.h"
#include "sound_svc.h"

static bool sound_on = true;

void buzzer_init(void) {
    sound_svc_init();
}

void buzzer_set_enabled(bool en) {
    sound_on = en;
    sound_svc_set_enabled(en);
}

bool buzzer_is_enabled(void) { return sound_on; }

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms) {
    sound_svc_request(freq_hz, duration_ms);
}

void buzzer_click(void)   { buzzer_beep(2000, 30); }
void buzzer_error(void)   { buzzer_beep(300, 300); }
void buzzer_success(void) { buzzer_beep(1000, 150); }

void buzzer_startup(void) {
    buzzer_beep(1000, 100);
    buzzer_beep(1500, 100);
    buzzer_beep(2000, 150);
}

void buzzer_tone_on(uint16_t freq_hz) { sound_svc_tone_on(freq_hz); }
void buzzer_tone_off(void)            { sound_svc_tone_off(); }
