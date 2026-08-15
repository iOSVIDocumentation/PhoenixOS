#ifndef BOARD_H
#define BOARD_H
/* PhoenixOS: единая карта железа. ВСЕ пины только здесь. */

/* Дисплей ST7789 (SPI0) */
#define PIN_LCD_SCK  18
#define PIN_LCD_MOSI 19
#define PIN_LCD_DC   16
#define PIN_LCD_CS   17
#define PIN_LCD_RST  20
#define PIN_LCD_BLK  21

/* SD-карта (SPI1) */
#define PIN_SD_SCK  10
#define PIN_SD_MOSI 11
#define PIN_SD_MISO 12
#define PIN_SD_CS   13

/* Джойстик */
#define JOY_X_PIN  26
#define JOY_Y_PIN  27
#define JOY_SW_PIN 5

/* Кнопки */
#define BTN_START 2
#define BTN_OK    3
#define BTN_BACK  4
#define BTN_COUNT 3

/* Буззер */
#define PIN_BUZZER 7

#endif // BOARD_H
