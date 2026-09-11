#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void snd_init(void);
void snd_beep(uint16_t freq_hz, uint32_t duration_ms);
void snd_click(void);
void snd_error(void);
void snd_success(void);

#endif // SOUND_H
