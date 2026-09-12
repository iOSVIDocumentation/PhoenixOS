#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "board.h"

#define LCD_WIDTH   320
#define LCD_HEIGHT  240
#define LCD_SPI_PORT spi0

#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_WIN_BG      0x0010
#define COLOR_WIN_GRAY    0xC618
#define COLOR_DARK_GRAY   0x4A49
#define COLOR_LIGHT_BLUE  0x3A9F
#define COLOR_YELLOW      0xFFE0

void st7789_init(void);
void st7789_set_backlight(uint8_t percent);
void st7789_fill(uint16_t color);
void st7789_draw_pixel(int16_t x, int16_t y, uint16_t color);
void st7789_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void st7789_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void st7789_draw_char(int16_t x, int16_t y, char ch, uint16_t color, uint16_t bg, uint8_t scale);
void st7789_draw_string(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t scale);
void st7789_draw_string_fast(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t max_chars);
void st7789_write_row(int16_t x, int16_t y, int16_t w, const uint8_t *buf);
void st7789_write_full_frame(const uint8_t *buf);
uint8_t st7789_font_row(char ch, uint8_t row);
void st7789_display_on(void);

#endif // ST7789_H
