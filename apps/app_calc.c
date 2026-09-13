#include "phoenix_core.h"
#include "st7789.h"
#include "ui.h"
#include "joystick.h"
#include "buttons.h"
#include "buzzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CALC_COLS 4
#define CALC_ROWS 4
#define CALC_BTN_W 60
#define CALC_BTN_H 40
#define CALC_DISP_H 32
#define CALC_DISP_Y 20

// Порог для джойстика (из диапазона 0-4095, центр ~2048)
#define CALC_JOY_LOW  1500
#define CALC_JOY_HIGH 2500
#define CALC_NAV_DELAY_MS 180

static const char *btn_labels[CALC_ROWS][CALC_COLS] = {
    {"7", "8", "9", "/"},
    {"4", "5", "6", "*"},
    {"1", "2", "3", "-"},
    {"C", "0", "=", "+"}
};

typedef struct {
    int cursor_row;
    int cursor_col;
    char expr_buf[64];
    int expr_len;
    double result_val;
    bool show_result;
    bool has_error;
    int last_cursor_row;
    int last_cursor_col;
    bool prev_ok;
    bool prev_back;
    bool prev_start;
    uint32_t last_nav_time;
} calc_state_t;

static calc_state_t *state = NULL;

static void calc_reset(void) {
    state->expr_buf[0] = 0;
    state->expr_len = 0;
    state->result_val = 0.0;
    state->show_result = false;
    state->has_error = false;
}

static void calc_add_char(char c) {
    if (state->expr_len < 62) {
        state->expr_buf[state->expr_len++] = c;
        state->expr_buf[state->expr_len] = 0;
    }
}

static double calc_evaluate(void) {
    char *s = state->expr_buf;
    double total = 0;
    double current = 0;
    char op = '+';
    bool has_num = false;

    while (*s) {
        if (*s >= '0' && *s <= '9') {
            current = current * 10 + (*s - '0');
            has_num = true;
        } else if (*s == '+' || *s == '-' || *s == '*' || *s == '/') {
            if (!has_num) return 0;
            switch (op) {
                case '+': total += current; break;
                case '-': total -= current; break;
                case '*': total *= current; break;
                case '/':
                    if (current == 0) { state->has_error = true; return 0; }
                    total /= current;
                    break;
            }
            op = *s;
            current = 0;
            has_num = false;
        }
        s++;
    }

    if (has_num) {
        switch (op) {
            case '+': total += current; break;
            case '-': total -= current; break;
            case '*': total *= current; break;
            case '/':
                if (current == 0) { state->has_error = true; return 0; }
                total /= current;
                break;
        }
    }

    return total;
}

static void calc_press(int r, int c) {
    const char *label = btn_labels[r][c];

    if (state->show_result) {
        calc_reset();
    }

    if (label[0] >= '0' && label[0] <= '9') {
        calc_add_char(label[0]);
    } else if (strcmp(label, "C") == 0) {
        calc_reset();
    } else if (strcmp(label, "=") == 0) {
        if (state->expr_len > 0) {
            state->result_val = calc_evaluate();
            state->show_result = true;
        }
    } else if (label[0] == '+' || label[0] == '-' || label[0] == '*' || label[0] == '/') {
        if (state->expr_len > 0 && state->expr_buf[state->expr_len - 1] != '+' &&
            state->expr_buf[state->expr_len - 1] != '-' &&
            state->expr_buf[state->expr_len - 1] != '*' &&
            state->expr_buf[state->expr_len - 1] != '/') {
            calc_add_char(label[0]);
        }
    }

    buzzer_click();
}

static void calc_draw_button(int r, int c, bool selected) {
    int x = 40 + c * CALC_BTN_W;
    int y = CALC_DISP_Y + CALC_DISP_H + 8 + r * CALC_BTN_H;

    uint16_t bg = selected ? COLOR_BLUE : COLOR_WIN_GRAY;
    uint16_t fg = COLOR_WHITE;

    st7789_fill_rect(x, y, CALC_BTN_W - 2, CALC_BTN_H - 2, bg);
    st7789_draw_rect(x, y, CALC_BTN_W - 2, CALC_BTN_H - 2, COLOR_BLACK);

    int text_x = x + CALC_BTN_W / 2 - 4;
    int text_y = y + CALC_BTN_H / 2 - 4;
    st7789_draw_string_fast(text_x, text_y, btn_labels[r][c], fg, bg, 1);
}

static void calc_draw_display(void) {
    st7789_fill_rect(0, CALC_DISP_Y, 320, CALC_DISP_H, COLOR_WHITE);
    st7789_draw_rect(0, CALC_DISP_Y, 320, CALC_DISP_H, COLOR_BLACK);

    if (state->has_error) {
        st7789_draw_string_fast(8, CALC_DISP_Y + 10, "Error", COLOR_RED, COLOR_WHITE, 10);
    } else if (state->show_result) {
        char res_str[32];
        if (state->result_val == (int)state->result_val) {
            snprintf(res_str, sizeof(res_str), "%d", (int)state->result_val);
        } else {
            snprintf(res_str, sizeof(res_str), "%.6g", state->result_val);
        }
        st7789_draw_string_fast(8, CALC_DISP_Y + 10, res_str, COLOR_BLACK, COLOR_WHITE, 20);
    } else {
        if (state->expr_len == 0) {
            st7789_draw_string_fast(8, CALC_DISP_Y + 10, "0", COLOR_BLACK, COLOR_WHITE, 1);
        } else {
            st7789_draw_string_fast(8, CALC_DISP_Y + 10, state->expr_buf, COLOR_BLACK, COLOR_WHITE, 40);
        }
    }
}

static void calc_render_full(void) {
    st7789_fill(COLOR_BLACK);
    ui_draw_title_bar(0, 0, 320, "Calc");
    calc_draw_display();
    for (int r = 0; r < CALC_ROWS; r++) {
        for (int c = 0; c < CALC_COLS; c++) {
            calc_draw_button(r, c, r == state->cursor_row && c == state->cursor_col);
        }
    }
    state->last_cursor_row = state->cursor_row;
    state->last_cursor_col = state->cursor_col;
}

static void calc_render_delta(void) {
    calc_draw_display();
    if (state->cursor_row != state->last_cursor_row || state->cursor_col != state->last_cursor_col) {
        calc_draw_button(state->last_cursor_row, state->last_cursor_col, false);
        calc_draw_button(state->cursor_row, state->cursor_col, true);
        state->last_cursor_row = state->cursor_row;
        state->last_cursor_col = state->cursor_col;
    }
}

static void calc_on_enter(int arg) {
    (void)arg;
    state = (calc_state_t *)malloc(sizeof(calc_state_t));
    if (!state) {
        core_open(APP_DESKTOP, 0);
        return;
    }

    state->cursor_row = 0;
    state->cursor_col = 0;
    state->last_cursor_row = -1;
    state->last_cursor_col = -1;
    state->prev_ok = false;
    state->prev_back = false;
    state->prev_start = false;
    state->last_nav_time = 0;

    calc_reset();
    calc_render_full();
}

static void calc_on_tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;

    if (!state) return;

    bool ok_edge = in->ok_pressed && !state->prev_ok;
    bool back_edge = in->back_pressed && !state->prev_back;
    bool start_edge = in->start_pressed && !state->prev_start;

    state->prev_ok = in->ok_pressed;
    state->prev_back = in->back_pressed;
    state->prev_start = in->start_pressed;

    if (back_edge || start_edge) {
        core_open(APP_DESKTOP, 0);
        buzzer_click();
        return;
    }

    // Навигация через raw.x/raw.y с большим порогом
    uint32_t now = core_now_ms();
    bool moved = false;

    if (now - state->last_nav_time >= CALC_NAV_DELAY_MS) {
        if (in->raw.x < CALC_JOY_LOW) {
            state->cursor_col = (state->cursor_col - 1 + CALC_COLS) % CALC_COLS;
            moved = true;
        } else if (in->raw.x > CALC_JOY_HIGH) {
            state->cursor_col = (state->cursor_col + 1) % CALC_COLS;
            moved = true;
        } else if (in->raw.y < CALC_JOY_LOW) {
            state->cursor_row = (state->cursor_row - 1 + CALC_ROWS) % CALC_ROWS;
            moved = true;
        } else if (in->raw.y > CALC_JOY_HIGH) {
            state->cursor_row = (state->cursor_row + 1) % CALC_ROWS;
            moved = true;
        }

        if (moved) {
            state->last_nav_time = now;
        }
    }

    if (moved) {
        buzzer_click();
        calc_render_delta();
        return;
    }

    if (ok_edge) {
        calc_press(state->cursor_row, state->cursor_col);
        calc_render_delta();
    }
}

static void calc_on_exit(void) {
    if (state) {
        free(state);
        state = NULL;
    }
}

const phoenix_app_t app_calc = {
    .name = "Calculator",
    .on_enter = calc_on_enter,
    .on_tick = calc_on_tick,
    .on_exit = calc_on_exit
};
