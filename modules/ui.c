#include "ui.h"
#include "wallpaper.h"
#include "theme.h"
#include "theme_icons.h"
#include "hardware/clocks.h"
#include <string.h>
#include <stdio.h>

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
    const theme_t *T = theme_get();
    if (wallpaper_is_loaded()) {
        wallpaper_blit_rect(x, y, w, h);
    } else {
        st7789_fill_rect(x, y, w, h, T->desktop);
    }
}

static void draw_title_bar(int x, int y, int w, const char *text) {
    const theme_t *T = theme_get();
    const int h = 16;
    st7789_fill_rect(x, y, w, h, T->title);
    if (T->title_stripes) {
        for (int yy = y + 3; yy <= y + h - 4; yy += 2) {
            st7789_fill_rect(x, yy, w, 1, COLOR_BLACK);
        }
        int tw = (int)strlen(text) * 8;
        int tx = x + (w - tw) / 2;
        st7789_fill_rect(tx - 3, y + 1, tw + 6, h - 2, T->title);
        st7789_draw_string(tx, y + 4, text, T->title_text, T->title, 1);
    } else {
        st7789_draw_string(x + 4, y + 4, text, T->title_text, T->title, 1);
    }
}

void ui_draw_title_bar(int x, int y, int w, const char *text) {
    draw_title_bar(x, y, w, text);
}

void ui_draw_bootscreen(bool safe_mode, int mount_state) {
    const theme_t *T = theme_get();
    st7789_fill(T->desktop);
    int x = 10, y = 10, w = 300, h = 220;
    st7789_fill_rect(x, y, w, h, T->win_bg);
    st7789_fill_rect(x, y, w, 1, T->win_light);
    st7789_fill_rect(x, y, 1, h, T->win_light);
    st7789_fill_rect(x, y + h - 1, w, 1, T->win_dark);
    st7789_fill_rect(x + w - 1, y, 1, h, T->win_dark);
    draw_title_bar(x + 2, y + 2, w - 4, "PhoenixOS");
    st7789_draw_string(72, 80, "Retro Workstation v0.9", T->text, T->win_bg, 1);
    st7789_draw_string(104, 95, "Kernel Edition", T->text, T->win_bg, 1);
    st7789_draw_string(68, 110, "RP2350 (ARM Cortex-M33)", COLOR_DARK_GRAY, T->win_bg, 1);
    if (safe_mode) {
        st7789_draw_string(60, 140, "SAFE MODE: settings reset", COLOR_RED, T->win_bg, 1);
    }
    if (mount_state < 0) {
        st7789_draw_string(45, 185, "Mounting SD (FAT32)...", T->text, T->win_bg, 1);
    } else if (mount_state == 1) {
        st7789_draw_string(45, 185, "SD Card Mounted! OK      ", COLOR_GREEN, T->win_bg, 1);
    } else {
        st7789_draw_string(45, 185, "SD Card Mount Failed!    ", COLOR_RED, T->win_bg, 1);
    }
}

static const char *cursor_map[UI_CURSOR_H] = {
    "B...........", "BB..........", "BWB.........", "BWWB........",
    "BWWWB.......", "BWWWWB......", "BWWWWWB.....", "BWWWWWWB....",
    "BWWWWWWWB...", "BWWWWWWWWB..", "BWWWWWWWWB..", "BWWWWWWWWB..",
    "BWWWWWWWB...", "BWWWWBWWB...", "BWWWB..BWB..", "BWWB....BWB.",
    "BWB......BB.", "BB.........."
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

/* ---------- Иконки: палитра из темы ---------- */

static void draw_icon_art(int x, int y, int icon_id, const char *const *art) {
    if (theme_icons_loaded()) {
        theme_icons_draw(icon_id, x, y);
    } else {
        const theme_t *T = theme_get();
        static const char *PAL = "KWGDBNYyPRE";
        for (int r = 0; r < 16; r++) {
            const char *line = art[r];
            for (int c = 0; c < 16; c++) {
                const char *pos = strchr(PAL, line[c]);
                if (!pos) continue;
                st7789_fill_rect(x + c * 2, y + r * 2, 2, 2, T->icol[pos - PAL]);
            }
        }
    }
}

static const char *const art_my_pc[] = {
    "................", ".KKKKKKKKKKKKKK.", ".KGGGGGGGGGGGGK.", ".KGBBBBBBBBBBGK.",
    ".KGBWWBBBBBBBGK.", ".KGBWWBBBBBBBGK.", ".KGBBBBBBBBBBGK.", ".KGBBWWWWWWBBGK.",
    ".KGBBBBBBBBBBGK.", ".KGGGGGGGGGGGGK.", ".KKKKKKKKKKKKKK.", "......KDDK......",
    "......KDDK......", "....KDDDDDDK....", "...KDDDDDDDDK...", "................"
};
static const char *const art_files[] = {
    "................", ".KKKK...........", ".KyyKKKKKKKKKKK.", ".KYYYYYYYYYYYYK.",
    ".KYWWWWWWWWWWYK.", ".KYWWWWWWWWWWYK.", ".KYYYYYYYYYYYYK.", ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.", ".KYYYYYYYYYYYYK.", ".KyyyyyyyyyyyyK.", ".KKKKKKKKKKKKKK.",
    "................", "................", "................", "................"
};
static const char *const art_settings[] = {
    "................", ".....KGGGGK.....", ".....KGGGGK.....", "..KKKGGGGGGKKK..",
    "..KGGGGGGGGGGK..", ".KGGGGKKKKGGGGK.", ".KGGGK....KGGGK.", ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.", ".KGGGK....KGGGK.", ".KGGGGKKKKGGGGK.", "..KGGGGGGGGGGK..",
    "..KKKGGGGGGKKK..", ".....KGGGGK.....", ".....KGGGGK.....", "................"
};
static const char *const art_games[] = {
    "................", "..KKKKKKKKKKKK..", ".KDDDDDDDDDDDDK.", ".KDDDWDDDDDRDDK.",
    ".KDWWWDDDDRRDDK.", ".KDDDWDDDDERDDK.", ".KDDDDDDDDDDDDK.", ".KDDDDDDDDDDDDK.",
    ".KDDKDDDDDDKDDK.", ".KKKDDDDDDDDKKK.", "..KKKKKKKKKKKK..", "................",
    "................", "................", "................", "................"
};
static const char *const art_media[] = {
    "................", ".KKKKKKKKKKKKKK.", ".KWKPKWKPKWKPWK.", ".KKKKKKKKKKKKKK.",
    ".KPPPPPPPPPPPPK.", ".KPPWPPPPPPPPPK.", ".KPPWWPPPPPPPPK.", ".KPPWWWPPPPPPPK.",
    ".KPPWWPPPPPPPPK.", ".KPPWPPPPPPPPPK.", ".KPPPPPPPPPPPPK.", ".KKKKKKKKKKKKKK.",
    "................", "................", "................", "................"
};

static uint8_t label_buf[64 * 2];

static void draw_label(int i) {
    const theme_t *T = theme_get();
    int len = (int)strlen(icon_labels[i]);
    int x = label_x[i];
    int y = 56;
    int w = len * 8;

    if (i == cur_hover) {
        st7789_fill_rect(x - 1, y - 1, w + 2, 10, COLOR_WHITE);
        st7789_draw_string_fast(x, y, icon_labels[i], T->desktop, COLOR_WHITE, (uint8_t)len);
        return;
    }

    uint8_t dh = (T->desktop >> 8) & 0xFF;
    uint8_t dl = T->desktop & 0xFF;
    for (int r = 0; r < 9; r++) {
        if (wallpaper_is_loaded()) {
            wallpaper_blit_to_buf(label_buf, x, y + r, w);
        } else {
            for (int p = 0; p < w; p++) {
                label_buf[2 * p]     = dh;
                label_buf[2 * p + 1] = dl;
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
        case ICON_MY_PC:     draw_icon_art(x, ICON_Y, ICON_MY_PC, art_my_pc);     break;
        case ICON_FILES:     draw_icon_art(x, ICON_Y, ICON_FILES, art_files);     break;
        case ICON_SETTINGS:  draw_icon_art(x, ICON_Y, ICON_SETTINGS, art_settings);  break;
        case ICON_GAMES:     draw_icon_art(x, ICON_Y, ICON_GAMES, art_games);     break;
        case ICON_MEDIA:     draw_icon_art(x, ICON_Y, ICON_MEDIA, art_media);     break;
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
    for (int i = 0; i < ICON_COUNT; i++) ui_draw_icon_cell(i);
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
        if (x < ix + iw && x + w > ix && y < iy + ih && y + h > iy) draw_icon_body(i);
        int lx = label_x[i] - 2, ly = 55;
        int lw = (int)strlen(icon_labels[i]) * 8 + 4, lh = 11;
        if (x < lx + lw && x + w > lx && y < ly + lh && y + h > ly) draw_label(i);
    }
}

int ui_hit_test(int cx, int cy) {
    for (int i = 0; i < ICON_COUNT; i++) {
        if (cx >= icon_x[i] && cx < icon_x[i] + ICON_SIZE && cy >= ICON_Y && cy < ICON_Y + ICON_SIZE) return i;
    }
    return -1;
}

void ui_draw_taskbar(void) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 216, 320, 24, T->taskbar);
    st7789_fill_rect(0, 216, 320, 1, T->win_light);
    st7789_fill_rect(0, 239, 320, 1, T->win_dark);

    st7789_fill_rect(2, 218, 50, 20, T->start_bg);
    st7789_draw_rect(2, 218, 50, 20, T->start_border);
    st7789_draw_string(10, 224, "Start", T->start_text, T->start_bg, 1);

    st7789_fill_rect(268, 218, 50, 20, T->taskbar);
    st7789_draw_rect(268, 218, 50, 20, T->win_dark);
    st7789_draw_string(275, 224, "12:00", T->taskbar_text, T->taskbar, 1);
}

void ui_draw_desktop(void) {
    const theme_t *T = theme_get();
    if (cur_wallpaper[0] && wallpaper_load(cur_wallpaper)) {
        wallpaper_blit_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    } else {
        wallpaper_unload();
        st7789_fill(T->desktop);
    }
    ui_draw_icons(-1);
    ui_draw_taskbar();
}

static const char *menu_items[UI_MENU_ITEMS] = {"Games", "About System", "Reboot"};
#define MENU_W      130
#define MENU_ITEM_H 18
#define MENU_PAD    2

void ui_draw_start_menu(int sel) {
    const theme_t *T = theme_get();
    int menu_h = UI_MENU_ITEMS * MENU_ITEM_H + MENU_PAD * 2;
    int x = 2;
    int y = 216 - menu_h;

    st7789_fill_rect(x, y, MENU_W, menu_h, T->win_bg);
    st7789_fill_rect(x, y, MENU_W, 1, T->win_light);
    st7789_fill_rect(x, y, 1, menu_h, T->win_light);
    st7789_fill_rect(x, y + menu_h - 1, MENU_W, 1, T->win_dark);
    st7789_fill_rect(x + MENU_W - 1, y, 1, menu_h, T->win_dark);

    for (int i = 0; i < UI_MENU_ITEMS; i++) {
        int iy = y + MENU_PAD + i * MENU_ITEM_H;
        if (i == sel) {
            st7789_fill_rect(x + MENU_PAD, iy, MENU_W - MENU_PAD * 2, MENU_ITEM_H, T->sel_bg);
            st7789_draw_string(x + MENU_PAD + 4, iy + 5, menu_items[i], T->sel_text, T->sel_bg, 1);
        } else {
            st7789_fill_rect(x + MENU_PAD, iy, MENU_W - MENU_PAD * 2, MENU_ITEM_H, T->win_bg);
            st7789_draw_string(x + MENU_PAD + 4, iy + 5, menu_items[i], T->text, T->win_bg, 1);
        }
    }
}

#include "sysinfo.h"
#include "phoenix_core.h"
void ui_draw_about_window(void) {
    const theme_t *T = theme_get();
    int w = 220, h = 170;
    int x = (320 - w) / 2;
    int y = (216 - h) / 2;

    st7789_fill_rect(x, y, w, h, T->win_bg);
    st7789_fill_rect(x, y, w, 1, T->win_light);
    st7789_fill_rect(x, y, 1, h, T->win_light);
    st7789_fill_rect(x, y + h - 1, w, 1, T->win_dark);
    st7789_fill_rect(x + w - 1, y, 1, h, T->win_dark);

    draw_title_bar(x + 3, y + 3, w - 6, "About PhoenixOS v0.9.5");

    int ty = y + 24;
    st7789_draw_string(x + 8, ty, "MCU: RP2350 M33 x2", T->text, T->win_bg, 1);
    {
        char clk[24];
        uint32_t hz = clock_get_hz(clk_sys);
        snprintf(clk, sizeof(clk), "Clock: %u MHz", (unsigned)((hz + 500000) / 1000000));
        st7789_draw_string(x + 8, ty + 12, clk, T->text, T->win_bg, 1);
    }
    {
        char ram[32];
        uint32_t total_kb, free_kb;
        sysinfo_ram_kb(&total_kb, &free_kb);
        snprintf(ram, sizeof(ram), "RAM: %lu/%lu KB", (unsigned long)free_kb, (unsigned long)total_kb);
        st7789_draw_string(x + 8, ty + 24, ram, T->text, T->win_bg, 1);
    }
    {
        char sd[40];
        uint32_t total_kb, free_kb;
        core_log("SDKB_PRE"); sysinfo_sd_kb(&total_kb, &free_kb); core_log("SDKB_POST");
        if (total_kb > 0) {
            snprintf(sd, sizeof(sd), "SD: %lu/%lu MB", (unsigned long)(free_kb / 1024), (unsigned long)(total_kb / 1024));
        } else {
            snprintf(sd, sizeof(sd), "SD: not mounted");
        }
        st7789_draw_string(x + 8, ty + 36, sd, T->text, T->win_bg, 1);
    }
    st7789_draw_string(x + 8, ty + 48, "LCD: 320x240 ST7789", T->text, T->win_bg, 1);
    st7789_draw_string(x + 8, ty + 60, "Sound: core1 service", T->text, T->win_bg, 1);
    st7789_draw_string(x + 8, ty + 72, "Watchdog: 1.5s", T->text, T->win_bg, 1);

    int bw = 40, bh = 16;
    int bx = x + (w - bw) / 2;
    int by = y + h - bh - 6;
    st7789_fill_rect(bx, by, bw, bh, T->win_bg);
    st7789_fill_rect(bx, by, bw, 1, T->win_light);
    st7789_fill_rect(bx, by, 1, bh, T->win_light);
    st7789_fill_rect(bx, by + bh - 1, bw, 1, T->win_dark);
    st7789_fill_rect(bx + bw - 1, by, 1, bh, T->win_dark);
    st7789_draw_string(bx + 14, by + 4, "OK", T->text, T->win_bg, 1);
}

static void draw_files_row(const fm_state_t *st, int idx, int sel, int y) {
    const theme_t *T = theme_get();
    uint16_t bg = (idx == sel) ? T->sel_bg : T->list_bg;
    uint16_t fg = (idx == sel) ? T->sel_text : T->list_text;
    st7789_fill_rect(3, y, 314, FM_ROW_H, bg);
    if (st->items[idx].is_dir) {
        st7789_fill_rect(6, y + 2, 8, 8, COLOR_YELLOW);
    } else {
        st7789_fill_rect(6, y + 2, 8, 8, T->list_bg);
        st7789_draw_rect(5, y + 1, 10, 10, (idx == sel) ? T->sel_text : T->win_dark);
    }
    st7789_draw_string_fast(20, y + 2, st->items[idx].name, fg, bg, 37);
}

void ui_files_row(const fm_state_t *st, int idx, int sel, int y) { draw_files_row(st, idx, sel, y); }

void ui_draw_files_window(const fm_state_t *st, int sel, int scroll) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);

    char title[48];
    int pl = (int)strlen(st->cwd);
    if (pl <= 30) {
        snprintf(title, sizeof(title), "Files - %s", st->cwd);
    } else {
        snprintf(title, sizeof(title), "Files - ...%s", st->cwd + pl - 27);
    }
    draw_title_bar(0, 0, 320, title);

    st7789_fill_rect(2, 18, 316, 196, T->list_bg);

    if (st->count == 0) {
        st7789_draw_string(20, 24, "(empty folder)", COLOR_DARK_GRAY, T->list_bg, 1);
        return;
    }

    for (int row = 0; row < FM_VISIBLE_ROWS; row++) {
        int idx = scroll + row;
        if (idx >= st->count) break;
        draw_files_row(st, idx, sel, 20 + row * FM_ROW_H);
    }
}

static const char *set_labels[SET_ROWS] = {"Brightness", "Cursor speed", "Sound", "Theme", "Wallpaper", "CPU Speed"};

static void draw_settings_row(int i, int sel, const settings_t *s) {
    const theme_t *T = theme_get();
    int y = 24 + i * 20;
    uint16_t bg = (i == sel) ? T->sel_bg : T->win_bg;
    uint16_t fg = (i == sel) ? T->sel_text : T->text;
    st7789_fill_rect(2, y, 316, 20, bg);
    st7789_draw_string_fast(8, y + 6, set_labels[i], fg, bg, 20);

    char val[32];
    switch (i) {
        case 0: snprintf(val, sizeof(val), "%u%%", s->brightness); break;
        case 1: snprintf(val, sizeof(val), "%s", s->cursor_speed == 1 ? "Slow" : s->cursor_speed == 2 ? "Normal" : "Fast"); break;
        case 2: snprintf(val, sizeof(val), "%s", s->sound_enabled ? "On" : "Off"); break;
        case 3: snprintf(val, sizeof(val), "%s", theme_name(s->theme)); break;
        case 4: {
            const char *sl = strrchr(s->wallpaper, '/');
            snprintf(val, sizeof(val), "%s", s->wallpaper[0] ? (sl ? sl + 1 : s->wallpaper) : "Standard");
            break;
        }
        default: snprintf(val, sizeof(val), "%u MHz", s->cpu_mhz); break;
    }
    st7789_draw_string_fast(180, y + 6, val, fg, bg, 17);
}

void ui_settings_row(int i, int sel, const settings_t *s) { draw_settings_row(i, sel, s); }

void ui_draw_settings_window(int sel, const settings_t *s) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);
    draw_title_bar(0, 0, 320, "Control Panel");
    for (int i = 0; i < SET_ROWS; i++) draw_settings_row(i, sel, s);
    st7789_draw_string_fast(4, 200, "L/R or OK: change  BACK: save", T->text, T->win_bg, 39);
}

static void draw_picker_row(int idx, int sel, const char *current, int y) {
    const theme_t *T = theme_get();
    uint16_t bg = (idx == sel) ? T->sel_bg : T->list_bg;
    uint16_t fg = (idx == sel) ? T->sel_text : T->list_text;
    st7789_fill_rect(3, y, 314, 18, bg);

    const char *name = (idx == 0) ? "(Standard)" : wallpaper_name(idx - 1);
    bool is_cur = (idx == 0 && current[0] == 0) || (idx > 0 && current[0] != 0 && strstr(current, name) != NULL);

    char line[WP_NAME_LEN + 4];
    snprintf(line, sizeof(line), "%c %s", is_cur ? '*' : ' ', name);
    st7789_draw_string_fast(8, y + 5, line, fg, bg, 37);
}

void ui_picker_row(int idx, int sel, const char *current, int y) { draw_picker_row(idx, sel, current, y); }

void ui_draw_wallpaper_picker(int sel, int scroll, const char *current) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);
    draw_title_bar(0, 0, 320, "Wallpapers");
    st7789_fill_rect(2, 18, 316, 180, T->list_bg);

    int total = wallpaper_count() + 1;
    for (int row = 0; row < WP_VISIBLE_ROWS; row++) {
        int idx = scroll + row;
        if (idx >= total) break;
        draw_picker_row(idx, sel, current, 20 + row * 18);
    }
    st7789_draw_string_fast(4, 200, "OK: apply  BACK: return", T->text, T->win_bg, 39);
}

static const char *cpu_labels[5] = {"150 MHz (Stock)", "200 MHz", "225 MHz", "250 MHz", "Continue"};
static const uint16_t cpu_mhz_vals[4] = {150, 200, 225, 250};

static void draw_cpu_row(int row, int sel, uint16_t current) {
    const theme_t *T = theme_get();
    int y = 20 + row * 20;
    uint16_t bg = (row == sel) ? T->sel_bg : T->list_bg;
    uint16_t fg = (row == sel) ? T->sel_text : T->list_text;
    st7789_fill_rect(3, y, 314, 20, bg);

    char line[24];
    if (row < 4) {
        snprintf(line, sizeof(line), "%c %s", (current == cpu_mhz_vals[row]) ? '*' : ' ', cpu_labels[row]);
    } else {
        snprintf(line, sizeof(line), "  %s", cpu_labels[row]);
    }
    st7789_draw_string_fast(8, y + 6, line, fg, bg, 37);
}

void ui_cpu_row(int row, int sel, uint16_t current) { draw_cpu_row(row, sel, current); }

void ui_draw_cpu_menu(int sel, uint16_t current) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);
    draw_title_bar(0, 0, 320, "CPU Speed");
    st7789_fill_rect(2, 18, 316, 104, T->list_bg);
    for (int row = 0; row < 5; row++) draw_cpu_row(row, sel, current);
    st7789_draw_string_fast(4, 200, "OK: apply  BACK: return", T->text, T->win_bg, 39);
}

/* ---------- Меню игр ---------- */

static const char *games_names[GAMES_ROWS] = {"Snake", "Tetris"};

void ui_games_row(int i, int sel, int y) {
    const theme_t *T = theme_get();
    uint16_t bg = (i == sel) ? T->sel_bg : T->list_bg;
    uint16_t fg = (i == sel) ? T->sel_text : T->list_text;
    st7789_fill_rect(3, y, 314, 20, bg);
    st7789_draw_string_fast(8, y + 6, games_names[i], fg, bg, 37);
}

void ui_draw_games_menu(int sel) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);
    draw_title_bar(0, 0, 320, "Games");
    st7789_fill_rect(2, 18, 316, 64, T->list_bg);
    for (int i = 0; i < GAMES_ROWS; i++) {
        ui_games_row(i, sel, 20 + i * 20);
    }
    st7789_draw_string_fast(4, 200, "OK: start  BACK: desktop", T->text, T->win_bg, 39);
}
