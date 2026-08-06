#include "phoenix_core.h"
#include "ui.h"
#include "files.h"
#include "viewer.h"
#include "buzzer.h"
#include <string.h>
#include <stdio.h>

static fm_state_t fm;
static int fm_sel = 0;
static int fm_scroll = 0;

static bool ends_with(const char *n, const char *ext) {
    int nl = (int)strlen(n);
    int el = (int)strlen(ext);
    if (nl < el + 1) return false;
    if (n[nl - el - 1] != '.') return false;
    for (int i = 0; i < el; i++) {
        char a = n[nl - el + i];
        char b = ext[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (a != b) return false;
    }
    return true;
}

static void files_enter(int arg) {
    if (arg == 0) {
        fm_init(&fm);
        fm_sel = 0;
        fm_scroll = 0;
    }
    ui_draw_files_window(&fm, fm_sel, fm_scroll);
}

static void files_tick(const core_input_t *in) {
    int old_sel = fm_sel;
    int old_scroll = fm_scroll;
    bool full = false;

    if (in->nav_up && fm_sel > 0) {
        fm_sel--;
        buzzer_click();
    } else if (in->nav_down && fm_sel < fm.count - 1) {
        fm_sel++;
        buzzer_click();
    }

    if (fm_sel < fm_scroll) fm_scroll = fm_sel;
    if (fm_sel >= fm_scroll + FM_VISIBLE_ROWS) fm_scroll = fm_sel - FM_VISIBLE_ROWS + 1;

    if (in->ok_pressed && fm.count > 0) {
        if (fm.items[fm_sel].is_dir) {
            if (fm_enter(&fm, fm_sel)) {
                fm_sel = 0;
                fm_scroll = 0;
                full = true;
                buzzer_click();
            } else {
                buzzer_error();
            }
        } else {
            char path[FM_PATH_LEN];
            if (strcmp(fm.cwd, "/") == 0) {
                snprintf(path, sizeof(path), "/%s", fm.items[fm_sel].name);
            } else {
                snprintf(path, sizeof(path), "%s/%s", fm.cwd, fm.items[fm_sel].name);
            }
            if (ends_with(fm.items[fm_sel].name, "pvx")) {
                buzzer_click();
                media_open_path(path);
                core_open(APP_MEDIA, 1);
                return;
            } else if (viewer_open(path)) {
                buzzer_click();
                core_open(APP_VIEWER, 0);
                return;
            } else {
                buzzer_error();
            }
        }
    }

    if (in->back_pressed) {
        buzzer_click();
        if (fm_go_up(&fm)) {
            fm_sel = 0;
            fm_scroll = 0;
            full = true;
        } else {
            core_open(APP_DESKTOP, 0);
            return;
        }
    }

    if (full || fm_scroll != old_scroll) {
        ui_draw_files_window(&fm, fm_sel, fm_scroll);
    } else if (fm_sel != old_sel) {
        ui_files_row(&fm, old_sel, fm_sel, 20 + (old_sel - fm_scroll) * FM_ROW_H);
        ui_files_row(&fm, fm_sel, fm_sel, 20 + (fm_sel - fm_scroll) * FM_ROW_H);
    }
}

const phoenix_app_t app_files = {
    .name = "files",
    .on_enter = files_enter,
    .on_tick = files_tick,
};
