#include "wallpaper.h"
#include "ff.h"
#include "st7789.h"
#include <string.h>

#define WP_MAX       32
#define WP_ROW_BYTES (LCD_WIDTH * 2)

static char wp_names[WP_MAX][WP_NAME_LEN];
static int wp_count = 0;
static uint8_t row_buf[WP_ROW_BYTES];

static uint8_t wp_cache[LCD_WIDTH * LCD_HEIGHT * 2];
static bool wp_loaded = false;

static bool ends_with_rgb(const char *n) {
    int nl = (int)strlen(n);
    if (nl < 5) return false;
    if (n[nl - 4] != '.') return false;
    for (int i = 0; i < 3; i++) {
        char a = n[nl - 3 + i];
        char b = "rgb"[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (a != b) return false;
    }
    return true;
}

int wallpaper_scan(void) {
    wp_count = 0;
    DIR dir;
    FILINFO fno;
    if (f_opendir(&dir, "/wallpapers") != FR_OK) return 0;
    while (wp_count < WP_MAX) {
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) break;
        if (fno.fattrib & AM_DIR) continue;
        if (!ends_with_rgb(fno.fname)) continue;
        strncpy(wp_names[wp_count], fno.fname, WP_NAME_LEN - 1);
        wp_names[wp_count][WP_NAME_LEN - 1] = 0;
        wp_count++;
    }
    f_closedir(&dir);

    for (int i = 1; i < wp_count; i++) {
        char key[WP_NAME_LEN];
        strcpy(key, wp_names[i]);
        int j = i - 1;
        while (j >= 0 && strcmp(wp_names[j], key) > 0) {
            strcpy(wp_names[j + 1], wp_names[j]);
            j--;
        }
        strcpy(wp_names[j + 1], key);
    }
    return wp_count;
}

int wallpaper_count(void) { return wp_count; }

const char *wallpaper_name(int i) {
    if (i < 0 || i >= wp_count) return "";
    return wp_names[i];
}

static void wp_cmd(uint8_t c) {
    gpio_put(PIN_LCD_DC, 0);
    gpio_put(PIN_LCD_CS, 0);
    spi_write_blocking(LCD_SPI_PORT, &c, 1);
    gpio_put(PIN_LCD_CS, 1);
}
static void wp_data(uint8_t d) {
    gpio_put(PIN_LCD_DC, 1);
    gpio_put(PIN_LCD_CS, 0);
    spi_write_blocking(LCD_SPI_PORT, &d, 1);
    gpio_put(PIN_LCD_CS, 1);
}

static void wp_window_row(int16_t x0, int16_t y, int16_t x1) {
    wp_cmd(0x2A);
    wp_data(x0 >> 8); wp_data(x0 & 0xFF);
    wp_data(x1 >> 8); wp_data(x1 & 0xFF);
    wp_cmd(0x2B);
    wp_data(y >> 8); wp_data(y & 0xFF);
    wp_data(y >> 8); wp_data(y & 0xFF);
    wp_cmd(0x2C);
}

bool wallpaper_draw(const char *path) {
    FIL f;
    if (f_open(&f, path, FA_READ) != FR_OK) return false;
    if (f_size(&f) < (FSIZE_t)(LCD_WIDTH * LCD_HEIGHT * 2)) {
        f_close(&f);
        return false;
    }
    for (int y = 0; y < LCD_HEIGHT; y++) {
        UINT br = 0;
        if (f_read(&f, row_buf, WP_ROW_BYTES, &br) != FR_OK || br != WP_ROW_BYTES) {
            f_close(&f);
            return false;
        }
        wp_window_row(0, y, LCD_WIDTH - 1);
        gpio_put(PIN_LCD_DC, 1);
        gpio_put(PIN_LCD_CS, 0);
        spi_write_blocking(LCD_SPI_PORT, row_buf, WP_ROW_BYTES);
        gpio_put(PIN_LCD_CS, 1);
    }
    f_close(&f);
    return true;
}

bool wallpaper_load(const char *path) {
    FIL f;
    if (f_open(&f, path, FA_READ) != FR_OK) return false;
    if (f_size(&f) < (FSIZE_t)(LCD_WIDTH * LCD_HEIGHT * 2)) {
        f_close(&f);
        return false;
    }
    UINT br = 0;
    FRESULT fr = f_read(&f, wp_cache, sizeof(wp_cache), &br);
    f_close(&f);
    if (fr != FR_OK || br != sizeof(wp_cache)) {
        wp_loaded = false;
        return false;
    }
    wp_loaded = true;
    return true;
}

void wallpaper_unload(void) {
    wp_loaded = false;
}

bool wallpaper_is_loaded(void) {
    return wp_loaded;
}

void wallpaper_blit_rect(int x, int y, int w, int h) {
    if (!wp_loaded) return;
    if (x < 0 || y < 0) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int row = y; row < y + h; row++) {
        wp_window_row(x, row, x + w - 1);
        gpio_put(PIN_LCD_DC, 1);
        gpio_put(PIN_LCD_CS, 0);
        spi_write_blocking(LCD_SPI_PORT,
                           &wp_cache[row * LCD_WIDTH * 2 + x * 2],
                           (size_t)w * 2);
        gpio_put(PIN_LCD_CS, 1);
    }
}

void wallpaper_blit_to_buf(uint8_t *dst, int x, int y, int w) {
    if (!wp_loaded || dst == NULL) return;
    if (x < 0 || y < 0 || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (w <= 0) return;
    memcpy(dst, &wp_cache[y * LCD_WIDTH * 2 + x * 2], (size_t)w * 2);
}
