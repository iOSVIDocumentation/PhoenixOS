#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"
#include <stdbool.h>
#include "board.h"

void buttons_init(void);
bool button_is_pressed(uint gpio);
bool button_was_pressed(uint gpio);

#endif // BUTTONS_H
