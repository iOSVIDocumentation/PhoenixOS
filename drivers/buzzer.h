#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include <stdbool.h>
#include "board.h"

void buzzer_init(void);
void buzzer_set_enabled(bool en);
bool buzzer_is_enabled(void);
void buzzer_beep(uint16_t frequency_hz, uint16_t duration_ms);
void buzzer_click(void);
void buzzer_error(void);
void buzzer_success(void);
void buzzer_startup(void);
void buzzer_tone_on(uint16_t freq_hz);
void buzzer_tone_off(void);

#endif // BUZZER_H
