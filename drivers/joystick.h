#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "board.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    bool sw_pressed;
} joystick_state_t;

void joystick_init(void);
void joystick_read(joystick_state_t *state);

#endif // JOYSTICK_H
