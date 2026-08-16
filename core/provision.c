#include "provision.h"
#include "theme_icons.h"
#include "ff.h"
#include <string.h>
#include <stdint.h>

static bool dir_exists(const char *path) {
    DIR d;
    if (f_opendir(&d, path) != FR_OK) return false;
    f_closedir(&d);
    return true;
}

static bool file_exists(const char *path) {
    FILINFO fi;
    return (f_stat(path, &fi) == FR_OK);
}

static void create_file(const char *path, const char *content) {
    FIL f;
    if (f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    UINT bw = 0;
    f_write(&f, content, (UINT)strlen(content), &bw);
    f_close(&f);
}

void provision_sd_card(void) {
    if (!dir_exists("/videos"))     f_mkdir("/videos");
    if (!dir_exists("/wallpapers")) f_mkdir("/wallpapers");
    if (!dir_exists("/themes"))         f_mkdir("/themes");
    if (!dir_exists("/themes/phoenix")) f_mkdir("/themes/phoenix");
    if (!dir_exists("/themes/xp"))      f_mkdir("/themes/xp");
    if (!dir_exists("/themes/macos"))   f_mkdir("/themes/macos");

    if (!file_exists("/config.txt")) {
        create_file("/config.txt", "System OK\n");
    }

    if (!file_exists("/snake.hi")) {
        create_file("/snake.hi", "classic=0 maze=0\n");
    }

    if (!file_exists("/phoenix.cfg")) {
        create_file("/phoenix.cfg",
                    "bright=100\n"
                    "cursor=2\n"
                    "sound=on\n"
                    "cpu=150\n"
                    "theme=0\n"
                    "wallpaper=\n");
    }

    /* иконки тем: Pico сам генерирует их на карту, если файлов нет */
    theme_icons_provision();
}
