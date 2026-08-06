#include "ui.h"
#include "wallpaper.h"
#include "hardware/clocks.h"
#include <string.h>
#include <stdio.h>

#define WIN_GRAY_BASE  0xC618
#define WIN_GRAY_LIGHT 0xF7DE
#define WIN_GRAY_DARK  0x8410
#define TITLE_BLUE     0x0015
#define YELLOW_DARK    0xCD00

#define ICON_Y    20
#define ICON_SIZE 32

static const int icon_x[ICON_COUNT] = {10, 72, 134, 196, 258};
static const int label_x[ICON_COUNT] = {6, 68, 118, 192, 254};
static const char *icon_labels[ICON_COUNT] = {"My PC", "Files", "Settings", "Games", "Media"};

static int cur_hover = -1;
static char cur_wallpaper[64] = "";

void ui_set_wallpaper(const char *path) {
    strncpy(cur_wallpaper, path, sizeof(cur_wallpaper) - 1);
    cur_wallpaper[sizeof(cur_wallpaper) - 1] = 0;
}

static void draw_bg_rect(int x, int y, int w, int h) {
    if (wallpaper_is_loaded()) {
        wallpaper_blit_rect(x, y, w, h);
    } else {
        st7789_fill_rect(x, y, w, h, COLOR_WIN_BG);
    }
}

/* ---------- Курсор-стрелка (12x18) ---------- */

static const char *cursor_map[UI_CURSOR_H] = {
    "B...........",
    "BB..........",
    "BWB.........",
    "BWWB........",
    "BWWWB.......",
    "BWWWWB......",
    "BWWWWWB.....",
    "BWWWWWWB....",
    "BWWWWWWWB...",
    "BWWWWWWWWB..",
    "BWWWWWWWWB..",
    "BWWWWWWWWB..",
    "BWWWWWWWB...",
    "BWWWWBWWB...",
    "BWWWB..BWB..",
    "BWWB....BWB.",
    "BWB......BB.",
    "BB..........",
};

void ui_draw_cursor(int x, int y) {
    for (int row = 0; row < UI_CURSOR_H; row++) {
        const char *line = cursor_map[row];
        int col = 0;
        while (col < UI_CURSOR_W) {
            char c = line[col];
            if (c == '.') { col++; continue; }
            int start = col;
            while (col < UI_CURSOR_W && line[col] == c) col++;
            uint16_t color = (c == 'B') ? COLOR_BLACK : COLOR_WHITE;
            st7789_fill_rect(x + start, y + row, col - start, 1, color);
        }
    }
}

/* ---------- Пиксель-арт иконок (16x16, масштаб x2) ---------- */

static void draw_icon_art(int x, int y, const char *const *art) {
    for (int r = 0; r < 16; r++) {
        const char *line = art[r];
        for (int c = 0; c < 16; c++) {
            char ch = line[c];
            uint16_t col;
            switch (ch) {
                case 'K': col = COLOR_BLACK; break;
                case 'W': col = COLOR_WHITE; break;
                case 'G': col = WIN_GRAY_LIGHT; break;
                case 'D': col = WIN_GRAY_DARK; break;
                case 'B': col = COLOR_LIGHT_BLUE; break;
                case 'N': col = 0x0013; break;
                case 'Y': col = COLOR_YELLOW; break;
                case 'y': col = YELLOW_DARK; break;
                case 'P': col = 0x780F; break;
                case 'R': col = COLOR_RED; break;
                case 'E': col = COLOR_GREEN; break;
                default: continue;
            }
            st7789_fill_rect(x + c * 2, y + r * 2, 2, 2, col);
        }
    }
}

static const char *const art_my_pc[] = {
    "................",
    ".KKKKKKKKKKKKKK.",
    ".KGGGGGGGGGGGGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGBWWBBBBBBBGK.",
    ".KGBWWBBBBBBBGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGBBWWWWWWBBGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGGGGGGGGGGGGK.",
    ".KKKKKKKKKKKKKK.",
    "......KDDK......",
    "......KDDK......",
    "....KDDDDDDK....",
    "...KDDDDDDDDK...",
    "................"
};

static const char *const art_files[] = {
    "................",
    ".KKKK...........",
    ".KyyKKKKKKKKKKK.",
    ".KYYYYYYYYYYYYK.",
    ".KYWWWWWWWWWWYK.",
    ".KYWWWWWWWWWWYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KyyyyyyyyyyyyK.",
    ".KKKKKKKKKKKKKK.",
    "................",
    "................",
    "................",
    "................"
};

static const char *const art_settings[] = {
    "................",
    ".....KGGGGK.....",
    ".....KGGGGK.....",
    "..KKKGGGGGGKKK..",
    "..KGGGGGGGGGGK..",
    ".KGGGGKKKKGGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGGKKKKGGGGK.",
    "..KGGGGGGGGGGK..",
    "..KKKGGGGGGKKK..",
    ".....KGGGGK.....",
    ".....KGGGGK.....",
    "................"
};

static const char *const art_games[] = {
    "................",
    "..KKKKKKKKKKKK..",
    ".KDDDDDDDDDDDDK.",
    ".KDDDWDDDDDRDDK.",
    ".KDWWWDDDDRRDDK.",
    ".KDDDWDDDDERDDK.",
    ".KDDDDDDDDDDDDK.",
    ".KDDDDDDDDDDDDK.",
    ".KDDKDDDDDDKDDK.",
    ".KKKDDDDDDDDKKK.",
    "..KKKKKKKKKKKK..",
    "................",
    "................",
    "................",
    "................",
    "................"
};

static const char *const art_media[] = {
    "................",
    ".KKKKKKKKKKKKKK.",
    ".KWKPKWKPKWKPWK.",
    ".KKKKKKKKKKKKKK.",
    ".KPPPPPPPPPPPPK.",
    ".KPPWPPPPPPPPPK.",
    ".KPPWWPPPPPPPPK.",
    ".KPPWWWPPPPPPPK.",
    ".KPPWWPPPPPPPPK.",
    ".KPPWPPPPPPPPPK.",
    ".KPPPPPPPPPPPPK.",
    ".KKKKKKKKKKKKKK.",
    "................",
    "................",
    "................",
    "................"
};

/* ---------- Подписи иконок (быстрая композиция в RAM) ---------- */

static uint8_t label_buf[64 * 2];

static void draw_label(int i) {
    int len = (int)strlen(icon_labels[i]);
    int x = label_x[i];
    int y = 56;
    int w = len * 8;

    if (i == cur_hover) {
        st7789_fill_rect(x - 1, y - 1, w + 2, 10, COLOR_WHITE);
        st7789_draw_string_fast(x, y, icon_labels[i], COLOR_WIN_BG, COLOR_WHITE, (uint8_t)len);
        return;
    }

    for (int r = 0; r < 9; r++) {
        if (wallpaper_is_loaded()) {
            wallpaper_blit_to_buf(label_buf, x, y + r, w);
        } else {
            for (int p = 0; p < w; p++) {
                label_buf[2 * p]     = COLOR_WIN_BG >> 8;
                label_buf[2 * p + 1] = COLOR_WIN_BG & 0xFF;
            }
        }
        for (int c = 0; c < len; c++) {
            uint8_t wb = (r <= 7) ? st7789_font_row(icon_labels[i][c], (uint8_t)r) : 0;
            uint8_t sb = (r >= 1) ? st7789_font_row(icon_labels[i][c], (uint8_t)(r - 1)) : 0;
            for (int j = 0; j < 8; j++) {
                int col = c * 8 + j;
                if (col >= w) break;
                if ((sb & (0x80 >> j)) && (col + 1 < w)) {
                    label_buf[2 * (col + 1)]     = 0x00;
                    label_buf[2 * (col + 1) + 1] = 0x00;
                }
                if (wb & (0x80 >> j)) {
                    label_buf[2 * col]     = 0xFF;
                    label_buf[2 * col + 1] = 0xFF;
                }
            }
        }
        st7789_write_row(x, y + r, w, label_buf);
    }
}

static void draw_icon_body(int i) {
    int x = icon_x[i];
    switch (i) {
        case ICON_MY_PC:     draw_icon_art(x, ICON_Y, art_my_pc);     break;
        case ICON_FILES:     draw_icon_art(x, ICON_Y, art_files);     break;
        case ICON_SETTINGS:  draw_icon_art(x, ICON_Y, art_settings);  break;
        case ICON_GAMES:     draw_icon_art(x, ICON_Y, art_games);     break;
        case ICON_MEDIA:     draw_icon_art(x, ICON_Y, art_media);     break;
    }
    if (i == cur_hover) {
        st7789_draw_rect(x - 2, ICON_Y - 2, ICON_SIZE + 4, ICON_SIZE + 4, COLOR_WHITE);
    }

}

void ui_draw_icon_cell(int i) {
    int x = icon_x[i];
    int zl = label_x[i] - 2;
    int zi = x - 3;
    int zx = (zl < zi) ? zl : zi;
    int rl = label_x[i] + (int)strlen(icon_labels[i]) * 8 + 2;
    int ri = x + ICON_SIZE + 3;
    int zr = (rl > ri) ? rl : ri;
    draw_bg_rect(zx, ICON_Y - 3, zr - zx, 48);
    draw_icon_body(i);
    draw_label(i);
}

void ui_draw_icons(int hover_id) {
    cur_hover = hover_id;
    for (int i = 0; i < ICON_COUNT; i++) {
        ui_draw_icon_cell(i);
    }
}

void ui_refresh_hover(int new_hover) {
    if (new_hover == cur_hover) return;
    int old = cur_hover;
    cur_hover = new_hover;
    if (old >= 0) ui_draw_icon_cell(old);
    if (new_hover >= 0) ui_draw_icon_cell(new_hover);
}

void ui_invalidate_rect(int x, int y, int w, int h) {
    draw_bg_rect(x, y, w, h);
    for (int i = 0; i < ICON_COUNT; i++) {
        int ix = icon_x[i] - 3, iy = ICON_Y - 3;
        int iw = ICON_SIZE + 6, ih = ICON_SIZE + 6;
        if (x < ix + iw && x + w > ix && y < iy + ih && y + h > iy) {
            draw_icon_body(i);
        }
        int lx = label_x[i] - 2, ly = 55;
        int lw = (int)strlen(icon_labels[i]) * 8 + 4, lh = 11;
        if (x < lx + lw && x + w > lx && y < ly + lh && y + h > ly) {
            draw_label(i);
        }
    }
}

int ui_hit_test(int cx, int cy) {
    for (int i = 0; i < ICON_COUNT; i++) {
        if (cx >= icon_x[i] && cx < icon_x[i] + ICON_SIZE &&
            cy >= ICON_Y && cy < ICON_Y + ICON_SIZE) {
            return i;
            }
    }
    return -1;
}

/* ---------- Рабочий стол ---------- */

void ui_draw_taskbar(void) {
    st7789_fill_rect(0, 216, 320, 24, WIN_GRAY_BASE);
    st7789_fill_rect(0, 216, 320, 1, WIN_GRAY_LIGHT);
    st7789_fill_rect(0, 239, 320, 1, WIN_GRAY_DARK);

    st7789_fill_rect(2, 218, 50, 20, WIN_GRAY_BASE);
    st7789_draw_rect(2, 218, 50, 20, COLOR_WHITE);
    st7789_draw_string(10, 224, "Start", COLOR_BLACK, WIN_GRAY_BASE, 1);

    st7789_fill_rect(268, 218, 50, 20, WIN_GRAY_BASE);
    st7789_draw_rect(268, 218, 50, 20, WIN_GRAY_DARK);
    st7789_draw_string(275, 224, "12:00", COLOR_BLACK, WIN_GRAY_BASE, 1);
}

void ui_draw_desktop(void) {
    if (cur_wallpaper[0] && wallpaper_load(cur_wallpaper)) {
        wallpaper_blit_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    } else {
        wallpaper_unload();
        st7789_fill(COLOR_WIN_BG);
    }
    ui_draw_icons(-1);
    ui_draw_taskbar();
}

/* ---------- Меню "Пуск" ---------- */

static const char *menu_items[UI_MENU_ITEMS] = {
    "About System",
    "Reboot",
};

#define MENU_W      130
#define MENU_ITEM_H 18
#define MENU_PAD    2

void ui_draw_start_menu(int sel) {
    int menu_h = UI_MENU_ITEMS * MENU_ITEM_H + MENU_PAD * 2;
    int x = 2;
    int y = 216 - menu_h;

    st7789_fill_rect(x, y, MENU_W, menu_h, WIN_GRAY_BASE);
    st7789_fill_rect(x, y, MENU_W, 1, WIN_GRAY_LIGHT);
    st7789_fill_rect(x, y, 1, menu_h, WIN_GRAY_LIGHT);
    st7789_fill_rect(x, y + menu_h - 1, MENU_W, 1, WIN_GRAY_DARK);
    st7789_fill_rect(x + MENU_W - 1, y, 1, menu_h, WIN_GRAY_DARK);

    for (int i = 0; i < UI_MENU_ITEMS; i++) {
        int iy = y + MENU_PAD + i * MENU_ITEM_H;
        if (i == sel) {
            st7789_fill_rect(x + MENU_PAD, iy, MENU_W - MENU_PAD * 2, MENU_ITEM_H, TITLE_BLUE);
            st7789_draw_string(x + MENU_PAD + 4, iy + 5, menu_items[i], COLOR_WHITE, TITLE_BLUE, 1);
        } else {
            st7789_fill_rect(x + MENU_PAD, iy, MENU_W - MENU_PAD * 2, MENU_ITEM_H, WIN_GRAY_BASE);
            st7789_draw_string(x + MENU_PAD + 4, iy + 5, menu_items[i], COLOR_BLACK, WIN_GRAY_BASE, 1);
        }
    }
}

/* ---------- Окно "О системе" (частота - РЕАЛЬНАЯ) ---------- */

void ui_draw_about_window(void) {
    int w = 210, h = 150;
    int x = (320 - w) / 2;
    int y = (216 - h) / 2;

    st7789_fill_rect(x, y, w, h, WIN_GRAY_BASE);
    st7789_fill_rect(x, y, w, 1, WIN_GRAY_LIGHT);
    st7789_fill_rect(x, y, 1, h, WIN_GRAY_LIGHT);
    st7789_fill_rect(x, y + h - 1, w, 1, WIN_GRAY_DARK);
    st7789_fill_rect(x + w - 1, y, 1, h, WIN_GRAY_DARK);

    st7789_fill_rect(x + 3, y + 3, w - 6, 14, TITLE_BLUE);
    st7789_draw_string(x + 6, y + 6, "About PhoenixOS", COLOR_WHITE, TITLE_BLUE, 1);

    int ty = y + 24;
    st7789_draw_string(x + 8, ty, "PhoenixOS v0.9 Kernel", COLOR_BLACK, WIN_GRAY_BASE, 1);
    st7789_draw_string(x + 8, ty + 12, "MCU: RP2350 M33 x2", COLOR_BLACK, WIN_GRAY_BASE, 1);
    {
        char clk[24];
        uint32_t hz = clock_get_hz(clk_sys);
        snprintf(clk, sizeof(clk), "Clock: %u MHz", (unsigned)((hz + 500000) / 1000000));
        st7789_draw_string(x + 8, ty + 24, clk, COLOR_BLACK, WIN_GRAY_BASE, 1);
    }
    st7789_draw_string(x + 8, ty + 36, "RAM: 520 KB SRAM", COLOR_BLACK, WIN_GRAY_BASE, 1);
    st7789_draw_string(x + 8, ty + 48, "LCD: 320x240 ST7789", COLOR_BLACK, WIN_GRAY_BASE, 1);
    st7789_draw_string(x + 8, ty + 60, "Storage: SD FAT32", COLOR_BLACK, WIN_GRAY_BASE, 1);

    int bw = 40, bh = 16;
    int bx = x + (w - bw) / 2;
    int by = y + h - bh - 6;
    st7789_fill_rect(bx, by, bw, bh, WIN_GRAY_BASE);
    st7789_fill_rect(bx, by, bw, 1, WIN_GRAY_LIGHT);
    st7789_fill_rect(bx, by, 1, bh, WIN_GRAY_LIGHT);
    st7789_fill_rect(bx, by + bh - 1, bw, 1, WIN_GRAY_DARK);
    st7789_fill_rect(bx + bw - 1, by, 1, bh, WIN_GRAY_DARK);
    st7789_draw_string(bx + 14, by + 4, "OK", COLOR_BLACK, WIN_GRAY_BASE, 1);
}

/* ---------- Окно файлового менеджера ---------- */

static void draw_path_title(const char *path) {
    int len = (int)strlen(path);
    if (len <= 30) {
        st7789_draw_string(60, 4, path, COLOR_WHITE, TITLE_BLUE, 1);
    } else {
        st7789_draw_string(60, 4, "...", COLOR_WHITE, TITLE_BLUE, 1);
        st7789_draw_string(84, 4, path + len - 27, COLOR_WHITE, TITLE_BLUE, 1);
    }
}

static void draw_files_row(const fm_state_t *st, int idx, int sel, int y) {
    uint16_t bg = (idx == sel) ? TITLE_BLUE : COLOR_WHITE;
    uint16_t fg = (idx == sel) ? COLOR_WHITE : COLOR_BLACK;
    st7789_fill_rect(3, y, 314, FM_ROW_H, bg);
    if (st->items[idx].is_dir) {
        st7789_fill_rect(6, y + 2, 8, 8, COLOR_YELLOW);
    } else {
        st7789_fill_rect(6, y + 2, 8, 8, COLOR_WHITE);
        st7789_draw_rect(5, y + 1, 10, 10, (idx == sel) ? COLOR_BLACK : WIN_GRAY_DARK);
    }
    st7789_draw_string_fast(20, y + 2, st->items[idx].name, fg, bg, 37);
}

void ui_files_row(const fm_state_t *st, int idx, int sel, int y) {
    draw_files_row(st, idx, sel, y);
}

void ui_draw_files_window(const fm_state_t *st, int sel, int scroll) {
    st7789_fill_rect(0, 0, 320, 216, WIN_GRAY_BASE);
    st7789_fill_rect(0, 0, 320, 16, TITLE_BLUE);
    st7789_draw_string(4, 4, "Files -", COLOR_WHITE, TITLE_BLUE, 1);
    draw_path_title(st->cwd);

    st7789_fill_rect(2, 18, 316, 196, COLOR_WHITE);

    if (st->count == 0) {
        st7789_draw_string(20, 24, "(empty folder)", COLOR_DARK_GRAY, COLOR_WHITE, 1);
        return;
    }

    for (int row = 0; row < FM_VISIBLE_ROWS; row++) {
        int idx = scroll + row;
        if (idx >= st->count) break;
        draw_files_row(st, idx, sel, 20 + row * FM_ROW_H);
    }
}

/* ---------- Панель управления ---------- */

static const char *set_labels[SET_ROWS] = {"Brightness", "Cursor speed", "Sound", "Wallpaper", "CPU Speed"};

static void draw_settings_row(int i, int sel, const settings_t *s) {
    int y = 24 + i * 20;
    uint16_t bg = (i == sel) ? TITLE_BLUE : WIN_GRAY_BASE;
    uint16_t fg = (i == sel) ? COLOR_WHITE : COLOR_BLACK;
    st7789_fill_rect(2, y, 316, 20, bg);
    st7789_draw_string_fast(8, y + 6, set_labels[i], fg, bg, 20);

    char val[32];
    switch (i) {
        case 0:
            snprintf(val, sizeof(val), "%u%%", s->brightness);
            break;
        case 1:
            snprintf(val, sizeof(val), "%s", s->cursor_speed == 1 ? "Slow" : s->cursor_speed == 2 ? "Normal" : "Fast");
            break;
        case 2:
            snprintf(val, sizeof(val), "%s", s->sound_enabled ? "On" : "Off");
            break;
        case 3: {
            const char *sl = strrchr(s->wallpaper, '/');
            snprintf(val, sizeof(val), "%s", s->wallpaper[0] ? (sl ? sl + 1 : s->wallpaper) : "Standard");
            break;
        }
        default:
            snprintf(val, sizeof(val), "%u MHz", s->cpu_mhz);
            break;
    }
    st7789_draw_string_fast(180, y + 6, val, fg, bg, 17);
}

void ui_settings_row(int i, int sel, const settings_t *s) {
    draw_settings_row(i, sel, s);
}

void ui_draw_settings_window(int sel, const settings_t *s) {
    st7789_fill_rect(0, 0, 320, 216, WIN_GRAY_BASE);
    st7789_fill_rect(0, 0, 320, 16, TITLE_BLUE);
    st7789_draw_string(4, 4, "Control Panel", COLOR_WHITE, TITLE_BLUE, 1);

    for (int i = 0; i < SET_ROWS; i++) {
        draw_settings_row(i, sel, s);
    }

    st7789_draw_string_fast(4, 200, "L/R or OK: change  BACK: save", COLOR_BLACK, WIN_GRAY_BASE, 39);
}

/* ---------- Выбор обоев ---------- */

static void draw_picker_row(int idx, int sel, const char *current, int y) {
    uint16_t bg = (idx == sel) ? TITLE_BLUE : COLOR_WHITE;
    uint16_t fg = (idx == sel) ? COLOR_WHITE : COLOR_BLACK;
    st7789_fill_rect(3, y, 314, 18, bg);

    const char *name = (idx == 0) ? "(Standard)" : wallpaper_name(idx - 1);
    bool is_cur = (idx == 0 && current[0] == 0) ||
    (idx > 0 && current[0] != 0 && strstr(current, name) != NULL);

    char line[WP_NAME_LEN + 4];
    snprintf(line, sizeof(line), "%c %s", is_cur ? '*' : ' ', name);
    st7789_draw_string_fast(8, y + 5, line, fg, bg, 37);
}

void ui_picker_row(int idx, int sel, const char *current, int y) {
    draw_picker_row(idx, sel, current, y);
}

void ui_draw_wallpaper_picker(int sel, int scroll, const char *current) {
    st7789_fill_rect(0, 0, 320, 216, WIN_GRAY_BASE);
    st7789_fill_rect(0, 0, 320, 16, TITLE_BLUE);
    st7789_draw_string(4, 4, "Wallpapers", COLOR_WHITE, TITLE_BLUE, 1);

    st7789_fill_rect(2, 18, 316, 180, COLOR_WHITE);

    int total = wallpaper_count() + 1;
    for (int row = 0; row < WP_VISIBLE_ROWS; row++) {
        int idx = scroll + row;
        if (idx >= total) break;
        draw_picker_row(idx, sel, current, 20 + row * 18);
    }

    st7789_draw_string_fast(4, 200, "OK: apply  BACK: return", COLOR_BLACK, WIN_GRAY_BASE, 39);
}

/* ---------- Меню частоты CPU ---------- */

static const char *cpu_labels[5] = {
    "150 MHz (Stock)", "200 MHz", "225 MHz", "250 MHz", "Continue"
};
static const uint16_t cpu_mhz_vals[4] = {150, 200, 225, 250};

static void draw_cpu_row(int row, int sel, uint16_t current) {
    int y = 20 + row * 20;
    uint16_t bg = (row == sel) ? TITLE_BLUE : COLOR_WHITE;
    uint16_t fg = (row == sel) ? COLOR_WHITE : COLOR_BLACK;
    st7789_fill_rect(3, y, 314, 20, bg);

    char line[24];
    if (row < 4) {
        snprintf(line, sizeof(line), "%c %s", (current == cpu_mhz_vals[row]) ? '*' : ' ', cpu_labels[row]);
    } else {
        snprintf(line, sizeof(line), "  %s", cpu_labels[row]);
    }
    st7789_draw_string_fast(8, y + 6, line, fg, bg, 37);
}

void ui_cpu_row(int row, int sel, uint16_t current) {
    draw_cpu_row(row, sel, current);
}

void ui_draw_cpu_menu(int sel, uint16_t current) {
    st7789_fill_rect(0, 0, 320, 216, WIN_GRAY_BASE);
    st7789_fill_rect(0, 0, 320, 16, TITLE_BLUE);
    st7789_draw_string(4, 4, "CPU Speed", COLOR_WHITE, TITLE_BLUE, 1);

    st7789_fill_rect(2, 18, 316, 104, COLOR_WHITE);

    for (int row = 0; row < 5; row++) {
        draw_cpu_row(row, sel, current);
    }

    st7789_draw_string_fast(4, 200, "OK: apply  BACK: return", COLOR_BLACK, WIN_GRAY_BASE, 39);
}
