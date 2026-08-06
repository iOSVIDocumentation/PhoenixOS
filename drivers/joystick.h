#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"
#include "hardware/adc.h"

#define JOY_X_PIN  26  // ADC0
#define JOY_Y_PIN  27  // ADC1
#define JOY_SW_PIN 5   // Цифровая кнопка

typedef struct {
    uint16_t x;          // Значение 0 - 4095
    uint16_t y;          // Значение 0 - 4095
    bool sw_pressed;     // true, если кнопка нажата
} joystick_state_t;

void joystick_init(void);
void joystick_read(joystick_state_t *state);

#endif // JOYSTICK_H
