#include "phoenix_core.h"
#include "ui.h"
#include "theme.h"
#include "buzzer.h"
#include <string.h>
#include <stdio.h>

#define T_COLS 10
#define T_ROWS 20
#define T_CELL 10
#define T_OX   6
#define T_OY   22

#define ST_MODE 0
#define ST_PLAY 1
#define ST_OVER 2

#define HOLD_STEP_MS 150

static uint8_t board[T_ROWS][T_COLS];
static uint8_t shp[4];
static int cur_p, cur_x, cur_y, next_p;
static int score, lines_n;
static int state = ST_MODE;
static int m_sel = 0;
static int mode = 0;
static uint32_t grav_ms = 600;
static uint32_t acc_ms = 0;
static uint32_t rng_st = 12345;

static int hold_dir = 0;
static uint32_t hold_acc = 0;

static const uint8_t base_shapes[7][4] = {
    {0x0, 0xF, 0x0, 0x0},
    {0x6, 0x6, 0x0, 0x0},
    {0x0, 0xE, 0x4, 0x0},
    {0x0, 0x6, 0xC, 0x0},
    {0x0, 0xC, 0x6, 0x0},
    {0x8, 0xE, 0x0, 0x0},
    {0x2, 0xE, 0x0, 0x0},
};

static const uint16_t piece_cols[7] = {
    0x07FF, 0xFFE0, 0x780F, 0x07E0, 0xF800, 0x001F, 0xFC00
};

static int rand7(void) {
    rng_st = rng_st * 1103515245u + 12345u;
    return (int)((rng_st >> 16) % 7u);
}

static void shape_rotate(uint8_t dst[4], const uint8_t src[4]) {
    for (int r = 0; r < 4; r++) {
        uint8_t row = 0;
        for (int c = 0; c < 4; c++) {
            if (src[3 - c] & (0x8 >> r)) row |= (0x8 >> c);
        }
        dst[r] = row;
    }
}

static void load_shape(int p, int rot) {
    for (int i = 0; i < 4; i++) shp[i] = base_shapes[p][i];
    uint8_t tmp[4];
    for (int i = 0; i < rot; i++) {
        shape_rotate(tmp, shp);
        for (int k = 0; k < 4; k++) shp[k] = tmp[k];
    }
}

static bool collide(int px, int py, const uint8_t s[4]) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!(s[r] & (0x8 >> c))) continue;
            int bx = px + c, by = py + r;
            if (bx < 0 || bx >= T_COLS || by >= T_ROWS) return true;
            if (by >= 0 && board[by][bx]) return true;
        }
    }
    return false;
}

static void draw_block(int bx, int by, uint16_t col) {
    st7789_fill_rect(T_OX + bx * T_CELL + 1, T_OY + by * T_CELL + 1, T_CELL - 2, T_CELL - 2, col);
}

static void draw_field(void) {
    const theme_t *T = theme_get();
    st7789_fill_rect(T_OX, T_OY, T_COLS * T_CELL, T_ROWS * T_CELL, T->list_bg);
    for (int y = 0; y < T_ROWS; y++) {
        for (int x = 0; x < T_COLS; x++) {
            if (board[y][x]) draw_block(x, y, piece_cols[board[y][x] - 1]);
        }
    }
    if (state == ST_PLAY) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if ((shp[r] & (0x8 >> c)) && (cur_y + r) >= 0) {
                    draw_block(cur_x + c, cur_y + r, piece_cols[cur_p]);
                }
            }
        }
    }
}

static void draw_panel(void) {
    const theme_t *T = theme_get();
    char buf[24];
    st7789_fill_rect(112, 16, 208, 208, T->win_bg);
    st7789_draw_string(120, 26, "Score", T->text, T->win_bg, 1);
    snprintf(buf, sizeof(buf), "%d", score);
    st7789_draw_string(120, 38, buf, T->text, T->win_bg, 2);
    snprintf(buf, sizeof(buf), "Lines %d", lines_n);
    st7789_draw_string(120, 60, buf, T->text, T->win_bg, 1);
    st7789_draw_string(120, 72, mode == 0 ? "Easy" : "Medium", T->text, T->win_bg, 1);
    st7789_draw_string(120, 92, "Next", T->text, T->win_bg, 1);

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (base_shapes[next_p][r] & (0x8 >> c)) {
                st7789_fill_rect(122 + c * 8, 104 + r * 8, 6, 6, piece_cols[next_p]);
            }
        }
    }
}

static void draw_all(void) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 240, T->win_bg);
    ui_draw_title_bar(0, 0, 320, "Tetris");
    st7789_draw_rect(T_OX - 1, T_OY - 1, T_COLS * T_CELL + 2, T_ROWS * T_CELL + 2, T->win_dark);
    draw_field();
    draw_panel();
    st7789_fill_rect(0, 224, 320, 16, T->taskbar);
    st7789_draw_string(4, 228, "SW/MENU:rot OK:drop BACK:games", T->taskbar_text, T->taskbar, 1);
}

static void draw_mode_menu(int sel) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, 216, T->win_bg);
    ui_draw_title_bar(0, 0, 320, "Tetris - Mode");
    st7789_fill_rect(2, 18, 316, 64, T->list_bg);
    for (int i = 0; i < 2; i++) {
        int y = 20 + i * 20;
        uint16_t bg = (i == sel) ? T->sel_bg : T->list_bg;
        uint16_t fg = (i == sel) ? T->sel_text : T->list_text;
        st7789_fill_rect(3, y, 314, 20, bg);
        st7789_draw_string_fast(8, y + 6, i == 0 ? "Easy" : "Medium", fg, bg, 37);
    }
    st7789_draw_string_fast(4, 200, "OK: start  BACK: games", T->text, T->win_bg, 39);
}

static void draw_over(void) {
    const theme_t *T = theme_get();
    int w = 180, h = 90;
    int x = (320 - w) / 2, y = (240 - h) / 2;
    st7789_fill_rect(x, y, w, h, T->win_bg);
    st7789_fill_rect(x, y, w, 1, T->win_light);
    st7789_fill_rect(x, y, 1, h, T->win_light);
    st7789_fill_rect(x, y + h - 1, w, 1, T->win_dark);
    st7789_fill_rect(x + w - 1, y, 1, h, T->win_dark);
    ui_draw_title_bar(x + 2, y + 2, w - 4, "Game Over");
    char buf[24];
    snprintf(buf, sizeof(buf), "Score %d", score);
    st7789_draw_string(x + 10, y + 34, buf, T->text, T->win_bg, 1);
    st7789_draw_string(x + 10, y + 50, "OK: retry", T->text, T->win_bg, 1);
    st7789_draw_string(x + 10, y + 62, "BACK: games", T->text, T->win_bg, 1);
}

static void clear_lines(void) {
    int cleared = 0;
    for (int y = T_ROWS - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < T_COLS; x++) {
            if (!board[y][x]) { full = false; break; }
        }
        if (full) {
            for (int yy = y; yy > 0; yy--) memcpy(board[yy], board[yy - 1], T_COLS);
            memset(board[0], 0, T_COLS);
            cleared++;
            y++;
        }
    }
    if (cleared) {
        score += cleared * cleared * 100;
        lines_n += cleared;
        buzzer_success();
    }
}

static void spawn(void) {
    cur_p = next_p;
    next_p = rand7();
    load_shape(cur_p, 0);
    cur_x = 3;
    cur_y = -1;
    if (collide(cur_x, cur_y, shp)) {
        state = ST_OVER;
        buzzer_error();
        draw_over();
    }
}

static void lock_and_next(void) {
    bool over = false;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!(shp[r] & (0x8 >> c))) continue;
            int by = cur_y + r, bx = cur_x + c;
            if (by < 0) { over = true; continue; }
            board[by][bx] = (uint8_t)(cur_p + 1);
        }
    }
    clear_lines();
    if (over) {
        state = ST_OVER;
        buzzer_error();
        draw_over();
        return;
    }
    spawn();
}

static void start_game(int m) {
    mode = m;
    grav_ms = (m == 0) ? 500 : 280;
    memset(board, 0, sizeof(board));
    score = 0;
    lines_n = 0;
    acc_ms = 0;
    hold_dir = 0;
    hold_acc = 0;
    rng_st = core_now_ms() | 1;
    next_p = rand7();
    state = ST_PLAY;
    spawn();
    if (state == ST_PLAY) draw_all();
}

static void tetris_enter(int arg) {
    (void)arg;
    state = ST_MODE;
    m_sel = 0;
    draw_mode_menu(0);
}

static void tetris_tick(const core_input_t *in, uint32_t delta_ms) {
    if (state == ST_MODE) {
        if (in->nav_up && m_sel > 0) { m_sel--; buzzer_click(); draw_mode_menu(m_sel); }
        else if (in->nav_down && m_sel < 1) { m_sel++; buzzer_click(); draw_mode_menu(m_sel); }
        if (in->ok_pressed || in->start_pressed) { buzzer_click(); start_game(m_sel); }
        if (in->back_pressed) { buzzer_click(); core_open(core_find("games"), 0); }
        return;
    }

    if (state == ST_OVER) {
        if (in->ok_pressed || in->start_pressed) { buzzer_click(); start_game(mode); }
        else if (in->back_pressed) { buzzer_click(); core_open(core_find("games"), 0); }
        return;
    }

    /* ST_PLAY */
    if (in->back_pressed) {
        buzzer_click();
        core_open(core_find("games"), 0);
        return;
    }

    bool changed = false;

    /* Поворот: МЕНЮ или нажатие джойстика (SW) */
    if (in->start_pressed || in->sw_pressed) {
        uint8_t rot[4];
        shape_rotate(rot, shp);
        static const int kicks[3] = {0, -1, 1};
        for (int k = 0; k < 3; k++) {
            if (!collide(cur_x + kicks[k], cur_y, rot)) {
                cur_x += kicks[k];
                memcpy(shp, rot, 4);
                changed = true;
                buzzer_click();
                break;
            }
        }
    }

    /* Hard drop: только физическая кнопка OK (SW не считается) */
    if (in->ok_pressed && !in->sw_pressed) {
        while (!collide(cur_x, cur_y + 1, shp)) cur_y++;
        lock_and_next();
        changed = true;
        hold_dir = 0;
        hold_acc = 0;
    }

    /* Движение джойстиком + авто-повтор при удержании */
    int new_hold;
    if      (in->raw.x < 1800) new_hold = -1;
    else if (in->raw.x > 2200) new_hold =  1;
    else if (in->raw.y > 2200) new_hold =  2;
    else                        new_hold =  0;

    if (new_hold != hold_dir) {
        hold_dir = new_hold;
        hold_acc = 0;
        if (hold_dir == -1 && !collide(cur_x - 1, cur_y, shp)) { cur_x--; changed = true; }
        else if (hold_dir ==  1 && !collide(cur_x + 1, cur_y, shp)) { cur_x++; changed = true; }
        else if (hold_dir ==  2 && !collide(cur_x, cur_y + 1, shp)) { cur_y++; changed = true; }
    } else if (hold_dir != 0) {
        hold_acc += delta_ms;
        while (hold_acc >= HOLD_STEP_MS && state == ST_PLAY) {
            hold_acc -= HOLD_STEP_MS;
            if (hold_dir == -1 && !collide(cur_x - 1, cur_y, shp)) { cur_x--; changed = true; }
            else if (hold_dir ==  1 && !collide(cur_x + 1, cur_y, shp)) { cur_x++; changed = true; }
            else if (hold_dir ==  2 && !collide(cur_x, cur_y + 1, shp)) { cur_y++; changed = true; }
        }
    }

    /* Гравитация */
    acc_ms += delta_ms;
    while (acc_ms >= grav_ms && state == ST_PLAY) {
        acc_ms -= grav_ms;
        if (!collide(cur_x, cur_y + 1, shp)) {
            cur_y++;
        } else {
            lock_and_next();
            hold_dir = 0;
            hold_acc = 0;
        }
        changed = true;
    }

    if (changed && state == ST_PLAY) {
        draw_field();
        draw_panel();
    }
}

const phoenix_app_t app_tetris = {
    .name = "tetris",
    .on_enter = tetris_enter,
    .on_tick = tetris_tick,
};
