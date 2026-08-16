#ifndef THEME_H
#define THEME_H

#include <stdint.h>
#include <stdbool.h>

#define THEME_COUNT 3
#define THEME_PHOENIX 0
#define THEME_WINXP   1
#define THEME_MACOS   2

typedef struct {
    const char *name;
    uint16_t win_bg, win_light, win_dark;
    uint16_t title, title_text;
    uint16_t text;
    uint16_t list_bg, list_text;
    uint16_t sel_bg, sel_text;
    uint16_t desktop;
    uint16_t taskbar, taskbar_text;
    uint16_t start_bg, start_text, start_border;
    bool title_stripes;
    /* палитра иконок: K W G D B N Y y P R E */
    uint16_t icol[11];
} theme_t;

const theme_t *theme_get(void);
void theme_set(int id);
int  theme_id(void);
const char *theme_name(int id);

#endif // THEME_H
