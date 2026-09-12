#include "theme.h"
#include "st7789.h"

static const theme_t themes[THEME_COUNT] = {
    [THEME_PHOENIX] = {
        .name = "Phoenix",
        .win_bg = 0xC618, .win_light = 0xF7DE, .win_dark = 0x8410,
        .title = 0x0015, .title_text = COLOR_WHITE,
        .text = COLOR_BLACK,
        .list_bg = COLOR_WHITE, .list_text = COLOR_BLACK,
        .sel_bg = 0x0015, .sel_text = COLOR_WHITE,
        .desktop = 0x0010,
        .taskbar = 0xC618, .taskbar_text = COLOR_BLACK,
        .start_bg = 0xC618, .start_text = COLOR_BLACK, .start_border = COLOR_WHITE,
        .title_stripes = false,
        .icol = {0x0000,0xFFFF,0xF7DE,0x8410,0x3A9F,0x0013,0xFFE0,0xCD00,0x780F,0xF800,0x07E0},
    },
    [THEME_WINXP] = {
        .name = "Windows XP",
        .win_bg = 0xEF5B, .win_light = 0xFFFF, .win_dark = 0xAD53,
        .title = 0x02BC, .title_text = COLOR_WHITE,
        .text = COLOR_BLACK,
        .list_bg = COLOR_WHITE, .list_text = COLOR_BLACK,
        .sel_bg = 0x3358, .sel_text = COLOR_WHITE,
        .desktop = 0x3B74,
        .taskbar = 0x02BC, .taskbar_text = COLOR_WHITE,
        .start_bg = 0x2C45, .start_text = COLOR_WHITE, .start_border = COLOR_WHITE,
        .title_stripes = false,
        .icol = {0x0000,0xFFFF,0xEF5B,0xAD53,0x02BC,0x0012,0xFFE0,0xCD00,0x780F,0xF800,0x2C45},
    },
    [THEME_MACOS] = {
        .name = "Mac OS Classic",
        .win_bg = COLOR_WHITE, .win_light = COLOR_WHITE, .win_dark = COLOR_BLACK,
        .title = COLOR_WHITE, .title_text = COLOR_BLACK,
        .text = COLOR_BLACK,
        .list_bg = COLOR_WHITE, .list_text = COLOR_BLACK,
        .sel_bg = COLOR_BLACK, .sel_text = COLOR_WHITE,
        .desktop = 0x8410,
        .taskbar = 0xC618, .taskbar_text = COLOR_BLACK,
        .start_bg = COLOR_WHITE, .start_text = COLOR_BLACK, .start_border = COLOR_BLACK,
        .title_stripes = true,
        .icol = {0x0000,0xFFFF,0xC618,0x8410,0x0010,0x0010,0xFFE0,0xCD00,0x780F,0xF800,0x07E0},
    },
    [THEME_TERMINAL] = {
        .name = "CRT Terminal",
        .win_bg = 0x0000, .win_light = 0x7F11, .win_dark = 0x7F11,
        .title = 0x7F11, .title_text = 0x0000,
        .text = 0x7F11,
        .list_bg = 0x0000, .list_text = 0x7F11,
        .sel_bg = 0x7F11, .sel_text = 0x0000,
        .desktop = 0x0000,
        .taskbar = 0x0000, .taskbar_text = 0x7F11,
        .start_bg = 0x0000, .start_text = 0x7F11, .start_border = 0x7F11,
        .title_stripes = true,
        .icol = {0x0000,0x7F11,0x7F11,0x0000,0x3B88,0x0000,0x7F11,0x3B88,0x7F11,0x7F11,0x7F11},
    },
};

static int cur_theme = THEME_PHOENIX;

const theme_t *theme_get(void) { return &themes[cur_theme]; }
int theme_id(void) { return cur_theme; }
void theme_set(int id) { if (id >= 0 && id < THEME_COUNT) cur_theme = id; }
const char *theme_name(int id) { return (id >= 0 && id < THEME_COUNT) ? themes[id].name : ""; }
