#include "hw_config.h"

// Настройка шины SPI1 (Пины 10, 11, 12, 13 на Pico 2)
static spi_t spi = {
    .hw_inst = spi1,
    .miso_gpio = 12,
    .mosi_gpio = 11,
    .sck_gpio = 10,
    .baud_rate = 12500000 // 12.5 МГц
};

static sd_card_t sd_card = {
    .pcName = "0:",
    .spi = &spi,
    .ss_gpio = 13,
    .use_card_detect = false
};

// Функции для работы с SD-картами
size_t sd_get_num(void) {
    return 1;
}

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) return &sd_card;
    return 0;
}

// Функции для работы с SPI-шинами (необходимы для FatFs_SPI)
size_t spi_get_num(void) {
    return 1;
}

spi_t *spi_get_by_num(size_t num) {
    if (0 == num) return &spi;
    return 0;
}
