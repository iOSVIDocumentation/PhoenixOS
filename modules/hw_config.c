#include "hw_config.h"
#include "board.h"

static spi_t spi = {
    .hw_inst = spi1,
    .miso_gpio = PIN_SD_MISO,
    .mosi_gpio = PIN_SD_MOSI,
    .sck_gpio = PIN_SD_SCK,
    .baud_rate = 12500000  /* 25 МГц твоя разводка не держит - не поднимать! */
};

static sd_card_t sd_card = {
    .pcName = "0:",
    .spi = &spi,
    .ss_gpio = PIN_SD_CS,
    .use_card_detect = false
};

size_t sd_get_num(void) { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) return &sd_card;
    return 0;
}

size_t spi_get_num(void) { return 1; }

spi_t *spi_get_by_num(size_t num) {
    if (0 == num) return &spi;
    return 0;
}
