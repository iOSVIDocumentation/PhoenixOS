#ifndef SOUND_SVC_H
#define SOUND_SVC_H

#include <stdint.h>
#include <stdbool.h>

/* Инициализация (вызывать с core0 до запуска core1) */
void sound_svc_init(void);

/* core0: заявки в очередь (неблокирующе) */
void sound_svc_request(uint16_t freq_hz, uint16_t dur_ms); /* dur>0: тон на время; freq=0,dur>0: пауза */
void sound_svc_tone_on(uint16_t freq_hz);                  /* постоянный тон */
void sound_svc_tone_off(void);                             /* стоп */
void sound_svc_set_enabled(bool en);

/* core1: движок, вызывать каждый виток core1_worker */
void sound_svc_core1_tick(bool movie_audio_active);

/* core1: нота из видео (приоритет над UI) */
void sound_svc_movie_note(uint16_t freq_hz, uint16_t dur_ms);

#endif // SOUND_SVC_H
