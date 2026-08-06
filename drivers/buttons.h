#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"
#include <stdbool.h>

/* Схема управления PhoenixOS:
 * Кнопка 1 (GPIO2) - START: открыть/закрыть меню "Пуск"
 * Кнопка 2 (GPIO3) - OK:    выбрать/подтвердить
 * Кнопка 3 (GPIO4) - BACK:  назад/закрыть окно
 */
#define BTN_START 2
#define BTN_OK    3
#define BTN_BACK  4
#define BTN_COUNT 3

void buttons_init(void);
bool button_is_pressed(uint gpio);
bool button_was_pressed(uint gpio);

#endif // BUTTONS_H
