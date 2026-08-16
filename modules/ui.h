#ifndef UI_H
#define UI_H

#include "pico/stdlib.h"
#include "st7789.h"
#include "files.h"
#include "settings.h"

#define UI_MENU_ITEMS 3

#define ICON_MY_PC    0
#define ICON_FILES    1
#define ICON_SETTINGS 2
#define ICON_GAMES    3
#define ICON_MEDIA    4
#define ICON_COUNT    5

#define UI_CURSOR_W 12
#define UI_CURSOR_H 18

#define FM_ROW_H        12
#define FM_VISIBLE_ROWS 16

#define SET_ROWS 6

#define WP_VISIBLE_ROWS 9

#define GAMES_ROWS 2

void ui_draw_desktop(void);
void ui_draw_taskbar(void);
void ui_draw_icons(int hover_id);
void ui_draw_icon_cell(int i);
void ui_refresh_hover(int new_hover);
void ui_invalidate_rect(int x, int y, int w, int h);
void ui_draw_cursor(int x, int y);
int  ui_hit_test(int cursor_x, int cursor_y);

void ui_draw_start_menu(int sel);
void ui_draw_about_window(void);
void ui_draw_files_window(const fm_state_t *st, int sel, int scroll);

void ui_set_wallpaper(const char *path);
void ui_draw_settings_window(int sel, const settings_t *s);
void ui_settings_row(int i, int sel, const settings_t *s);
void ui_draw_wallpaper_picker(int sel, int scroll, const char *current);
void ui_picker_row(int idx, int sel, const char *current, int y);
void ui_draw_cpu_menu(int sel, uint16_t current);
void ui_cpu_row(int row, int sel, uint16_t current);
void ui_files_row(const fm_state_t *st, int idx, int sel, int y);

void ui_draw_title_bar(int x, int y, int w, const char *text);
void ui_draw_bootscreen(bool safe_mode, int mount_state);
void ui_draw_games_menu(int sel);
void ui_games_row(int i, int sel, int y);

#endif // UI_H
