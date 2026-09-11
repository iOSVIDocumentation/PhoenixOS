#include "sound.h"
#include "buzzer.h"

void snd_init(void) {
    buzzer_init();
}

void snd_beep(uint16_t freq_hz, uint32_t duration_ms) {
    buzzer_beep(freq_hz, duration_ms);
}

void snd_click(void) { buzzer_click(); }
void snd_error(void) { buzzer_error(); }
void snd_success(void) { buzzer_beep(600, 50); buzzer_beep(900, 100); }
