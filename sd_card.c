#include "sd_card.h"
#include "hw_config.h"
#include "ff.h"

static FATFS fs;
static bool mounted = false;

bool init_fatfs() {
    if (mounted) return true;

    FRESULT fr = f_mount(&fs, "", 1);
    if (fr == FR_OK) {
        mounted = true;
        return true;
    }
    return false;
}

bool read_text_file(const char* filename, char* buffer, UINT max_len) {
    FIL fil;
    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) return false;

    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) return false;

    UINT br;
    f_read(&fil, buffer, max_len - 1, &br);
    buffer[br] = '\0';

    f_close(&fil);
    return true;
}
