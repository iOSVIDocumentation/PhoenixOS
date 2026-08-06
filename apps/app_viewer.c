#include "phoenix_core.h"
#include "viewer.h"
#include "buzzer.h"

static int view_scroll = 0;

static void viewer_enter(int arg) {
    (void)arg;
    view_scroll = 0;
    viewer_draw_header();
    viewer_draw(view_scroll);
}

static void viewer_tick(const core_input_t *in) {
    bool need_redraw = false;

    int max_scroll = viewer_get_lines() - VIEWER_VISIBLE;
    if (max_scroll < 0) max_scroll = 0;

    if (in->nav_up && view_scroll > 0) {
        view_scroll--;
        need_redraw = true;
    } else if (in->nav_down && view_scroll < max_scroll) {
        view_scroll++;
        need_redraw = true;
    } else if (in->nav_left && view_scroll > 0) {
        view_scroll -= 10;
        if (view_scroll < 0) view_scroll = 0;
        need_redraw = true;
    } else if (in->nav_right && view_scroll < max_scroll) {
        view_scroll += 10;
        if (view_scroll > max_scroll) view_scroll = max_scroll;
        need_redraw = true;
    }

    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_FILES, 1);
        return;
    }

    if (need_redraw) {
        viewer_draw(view_scroll);
    }
}

const phoenix_app_t app_viewer = {
    .name = "viewer",
    .on_enter = viewer_enter,
    .on_tick = viewer_tick,
};
