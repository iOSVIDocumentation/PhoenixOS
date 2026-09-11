#include "phoenix_core.h"
#include "st7789.h"
#include "buzzer.h"
#include "theme.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

#define SN_CELL 8
#define SN_COLS 39
#define SN_ROWS 27
#define SN_OX 4
#define SN_OY 24
#define SN_MAXLEN (SN_COLS * SN_ROWS)

#define SN_GREEN     0x07E0
#define SN_GREEN_LT  0x37E6
#define SN_GREEN_DK  0x03A0

typedef struct { uint8_t x, y; } sn_pt_t;

static uint8_t maze[SN_ROWS][SN_COLS];
static sn_pt_t body[SN_MAXLEN];
static int sn_len = 0;
static int8_t dir_x = 1, dir_y = 0;
static sn_pt_t food;
static uint32_t score = 0;
static uint32_t best[2] = {0, 0};
static uint32_t step_ms = 160;
static uint32_t acc_ms = 0;
static int mode = 0;
static int state = 0;
static int menu_sel = 0;
static uint32_t game_no = 0;
static uint32_t rng = 123456789;

static uint32_t rnd(void) {
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
    return rng;
}

static void hi_save(void);

static void hi_load(void) {
    best[0] = best[1] = 0;
    FIL f;
    if (f_open(&f, "/snake.hi", FA_READ) != FR_OK) { hi_save(); return; }
    char buf[64]; UINT br = 0;
    f_read(&f, buf, sizeof(buf) - 1, &br);
    f_close(&f);
    buf[br] = 0;
    unsigned long a = 0, b = 0;
    sscanf(buf, "classic=%lu maze=%lu", &a, &b);
    best[0] = a; best[1] = b;
}

static void hi_save(void) {
    FIL f;
    if (f_open(&f, "/snake.hi", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "classic=%lu maze=%lu\n",
                     (unsigned long)best[0], (unsigned long)best[1]);
    UINT bw = 0;
    f_write(&f, buf, n, &bw);
    f_close(&f);
}

static void cell_floor(int x, int y) {

    uint16_t c = COLOR_BLACK; /* поле полностью чёрное */
    st7789_fill_rect(SN_OX + x * SN_CELL, SN_OY + y * SN_CELL, SN_CELL, SN_CELL, c);
}

static void cell_wall(int x, int y) {
    const theme_t *T = theme_get();
    int px = SN_OX + x * SN_CELL, py = SN_OY + y * SN_CELL;
    st7789_fill_rect(px, py, 8, 8, T->win_bg);
    st7789_fill_rect(px, py, 8, 1, T->win_light);
    st7789_fill_rect(px, py, 1, 8, T->win_light);
    st7789_fill_rect(px, py + 7, 8, 1, COLOR_BLACK);
    st7789_fill_rect(px + 7, py, 1, 8, COLOR_BLACK);
}

static void cell_fruit(int x, int y) {
    int px = SN_OX + x * SN_CELL + 1, py = SN_OY + y * SN_CELL + 2;
    st7789_fill_rect(px + 1, py, 4, 6, COLOR_RED);
    st7789_fill_rect(px, py + 1, 6, 4, COLOR_RED);
    st7789_fill_rect(px + 1, py + 1, 1, 1, COLOR_WHITE);
    st7789_fill_rect(px + 3, py - 1, 1, 1, COLOR_BLACK);
    st7789_fill_rect(px + 4, py - 1, 1, 1, COLOR_GREEN);
}

static void cell_body(int x, int y) {
    int px = SN_OX + x * SN_CELL, py = SN_OY + y * SN_CELL;
    st7789_fill_rect(px + 1, py + 1, 6, 6, SN_GREEN);
    st7789_fill_rect(px + 1, py + 1, 6, 1, SN_GREEN_LT);
    st7789_fill_rect(px + 1, py + 6, 6, 1, SN_GREEN_DK);
}

static void die(void);

static void cell_head(int x, int y) {
    int px = SN_OX + x * SN_CELL, py = SN_OY + y * SN_CELL;
    st7789_fill_rect(px, py, 8, 8, SN_GREEN);
    st7789_fill_rect(px, py, 8, 1, SN_GREEN_LT);
    st7789_fill_rect(px, py + 7, 8, 1, SN_GREEN_DK);
    int e1x = 1, e1y = 1, e2x = 5, e2y = 1;
    if (dir_x == 1)       { e1x = 5; e1y = 1; e2x = 5; e2y = 5; }
    else if (dir_x == -1) { e1x = 1; e1y = 1; e2x = 1; e2y = 5; }
    else if (dir_y == 1)  { e1x = 1; e1y = 5; e2x = 5; e2y = 5; }
    st7789_fill_rect(px + e1x, py + e1y, 2, 2, COLOR_BLACK);
    st7789_fill_rect(px + e2x, py + e2y, 2, 2, COLOR_BLACK);
}

static void hud(void) {
    const theme_t *T = theme_get();
    st7789_fill_rect(0, 0, 320, SN_OY - 2, T->taskbar);
    st7789_fill_rect(0, SN_OY - 3, 320, 1, T->win_dark);
    st7789_draw_string(4, 4, "SNAKE", T->taskbar_text, T->taskbar, 2);
    st7789_draw_string(120, 10, mode ? "MAZE" : "CLASSIC", T->taskbar_text, T->taskbar, 1);
    char buf[24];
    snprintf(buf, sizeof(buf), "SCORE %lu", (unsigned long)score);
    st7789_draw_string(316 - 8 * (int)strlen(buf), 10, buf, T->taskbar_text, T->taskbar, 1);
}

static void menu_draw(void) {
    const theme_t *T = theme_get();
    st7789_fill(T->desktop);
    int w = 220, h = 130, x = (320 - w) / 2, y = (216 - h) / 2;
    st7789_fill_rect(x, y, w, h, T->win_bg);
    st7789_fill_rect(x, y, w, 1, T->win_light);
    st7789_fill_rect(x, y, 1, h, T->win_light);
    st7789_fill_rect(x, y + h - 1, w, 1, T->win_dark);
    st7789_fill_rect(x + w - 1, y, 1, h, T->win_dark);
    st7789_fill_rect(x + 3, y + 3, w - 6, 14, T->title);
    st7789_draw_string(x + 8, y + 6, "SNAKE - choose mode", T->title_text, T->title, 1);

    const char *names[2] = { "CLASSIC", "MAZE" };
    for (int i = 0; i < 2; i++) {
        int ry = y + 30 + i * 26;
        uint16_t bg = (i == menu_sel) ? T->sel_bg : T->win_bg;
        uint16_t fg = (i == menu_sel) ? T->sel_text : T->text;
        st7789_fill_rect(x + 6, ry, w - 12, 20, bg);
        char line[40];
        snprintf(line, sizeof(line), "%s   BEST %lu", names[i], (unsigned long)best[i]);
        st7789_draw_string(x + 14, ry + 6, line, fg, bg, 1);
    }
    st7789_draw_string(x + 40, y + h - 18, "OK-start  BACK-exit", T->text, T->win_bg, 1);
}

static void draw_field(void) {
    for (int y = 0; y < SN_ROWS; y++)
        for (int x = 0; x < SN_COLS; x++)
            if (maze[y][x]) cell_wall(x, y); else cell_floor(x, y);
    for (int i = sn_len - 1; i >= 1; i--) cell_body(body[i].x, body[i].y);
    cell_head(body[0].x, body[0].y);
}

static uint32_t flood_count(int sx, int sy) {
    static uint8_t seen[SN_ROWS][SN_COLS];
    static sn_pt_t st[SN_MAXLEN];
    memset(seen, 0, sizeof(seen));
    int sp = 0; uint32_t cnt = 0;
    st[sp].x = (uint8_t)sx; st[sp].y = (uint8_t)sy; sp++;
    seen[sy][sx] = 1;
    const int8_t dx[4] = {1, -1, 0, 0};
    const int8_t dy[4] = {0, 0, 1, -1};
    while (sp > 0) {
        sn_pt_t c = st[--sp];
        cnt++;
        for (int i = 0; i < 4; i++) {
            int nx = c.x + dx[i], ny = c.y + dy[i];
            if (nx < 0 || nx >= SN_COLS || ny < 0 || ny >= SN_ROWS) continue;
            if (seen[ny][nx] || maze[ny][nx]) continue;
            seen[ny][nx] = 1;
            st[sp].x = (uint8_t)nx; st[sp].y = (uint8_t)ny; sp++;
        }
    }
    return cnt;
}

static void maze_gen(int cx, int cy) {
    for (int attempt = 0; attempt < 30; attempt++) {
        memset(maze, 0, sizeof(maze));
        int segs = 30 + (int)(rnd() % 12);
        for (int s = 0; s < segs; s++) {
            int x = 1 + (int)(rnd() % (SN_COLS - 2));
            int y = 1 + (int)(rnd() % (SN_ROWS - 2));
            int horiz = (int)(rnd() & 1);
            int len = 2 + (int)(rnd() % 4);
            for (int i = 0; i < len; i++) {
                int xx = horiz ? x + i : x;
                int yy = horiz ? y : y + i;
                if (xx < 0 || xx >= SN_COLS || yy < 0 || yy >= SN_ROWS) break;
                if (xx >= cx - 2 && xx <= cx + 2 && yy >= cy - 1 && yy <= cy + 1) continue;
                maze[yy][xx] = 1;
            }
        }
        uint32_t open = 0;
        for (int y = 0; y < SN_ROWS; y++)
            for (int x = 0; x < SN_COLS; x++)
                open += !maze[y][x];
        if (flood_count(cx, cy) == open) return;
    }
    memset(maze, 0, sizeof(maze));
}

static void food_place(void) {
    if (sn_len >= SN_MAXLEN) { die(); return; }  // WIN condition
    int attempts = 0;
    while (attempts < 1000) {
        int x = (int)(rnd() % SN_COLS), y = (int)(rnd() % SN_ROWS);
        if (maze[y][x]) { attempts++; continue; }
        bool on = false;
        for (int i = 0; i < sn_len; i++)
            if (body[i].x == x && body[i].y == y) { on = true; break; }
        if (on) { attempts++; continue; }
        food.x = (uint8_t)x; food.y = (uint8_t)y;
        cell_fruit(x, y);
        return;
    }
    die();  // fallback: если не нашли свободную клетку за 1000 попыток
}

static void die(void) {
    const theme_t *T = theme_get();
    state = 2;
    buzzer_error();
    bool record = score > best[mode];
    if (record) { best[mode] = score; hi_save(); }
    int w = 200, h = 76, x = (320 - w) / 2, y = (216 - h) / 2;
    st7789_fill_rect(x, y, w, h, T->win_bg);
    st7789_fill_rect(x, y, w, 1, T->win_light);
    st7789_fill_rect(x, y, 1, h, T->win_light);
    st7789_fill_rect(x, y + h - 1, w, 1, T->win_dark);
    st7789_fill_rect(x + w - 1, y, 1, h, T->win_dark);
    st7789_fill_rect(x + 3, y + 3, w - 6, 14, T->title);
    st7789_draw_string(x + 70, y + 6, "GAME OVER", T->title_text, T->title, 1);
    char buf[40];
    snprintf(buf, sizeof(buf), "SCORE %lu   BEST %lu", (unsigned long)score, (unsigned long)best[mode]);
    st7789_draw_string(x + 30, y + 26, buf, T->text, T->win_bg, 1);
    if (record)
        st7789_draw_string(x + 60, y + 40, "*** NEW RECORD! ***", COLOR_YELLOW, T->win_bg, 1);
    st7789_draw_string(x + 40, y + 56, "OK-retry  BACK-games", T->text, T->win_bg, 1);
}

static void game_start(int m) {
    mode = m;
    game_no++;
    rng = (uint32_t)to_us_since_boot(get_absolute_time()) ^ (0x9E3779B9u + game_no * 2654435761u);
    if (rng == 0) rng = 1;
    int cx = (SN_COLS / 2) | 1, cy = (SN_ROWS / 2) | 1;
    if (mode) {
        maze_gen(cx, cy);
    } else {
        memset(maze, 0, sizeof(maze));
    }
    body[0].x = (uint8_t)(cx + 1); body[0].y = (uint8_t)cy;
    body[1].x = (uint8_t)cx;       body[1].y = (uint8_t)cy;
    body[2].x = (uint8_t)(cx - 1); body[2].y = (uint8_t)cy;
    sn_len = 3;
    dir_x = 1; dir_y = 0;
    score = 0;
    step_ms = 160;
    acc_ms = 0;
    state = 1;
    hud();
    draw_field();
    food_place();
}

static void step(void) {
    int nx = body[0].x + dir_x, ny = body[0].y + dir_y;
    if (nx < 0 || nx >= SN_COLS || ny < 0 || ny >= SN_ROWS || maze[ny][nx]) { die(); return; }
    int ddx = nx - food.x, ddy = ny - food.y;
    if (ddx < 0) ddx = -ddx;
    if (ddy < 0) ddy = -ddy;
    bool eat = (ddx <= 1 && ddy <= 1);
    for (int i = 0; i < sn_len - (eat ? 0 : 1); i++) {
        if (body[i].x == nx && body[i].y == ny) { die(); return; }
    }
    sn_pt_t old_tail = body[sn_len - 1];
    sn_pt_t old_food = food;
    if (eat) {
        memmove(&body[1], &body[0], sizeof(sn_pt_t) * sn_len);
        if (sn_len < SN_MAXLEN) sn_len++;
        score += 10;
        if (step_ms > 70) step_ms -= 4;
        buzzer_click();
    } else {
        memmove(&body[1], &body[0], sizeof(sn_pt_t) * (sn_len - 1));
    }
    body[0].x = (uint8_t)nx; body[0].y = (uint8_t)ny;
    if (!eat) cell_floor(old_tail.x, old_tail.y);
    cell_body(body[1].x, body[1].y);
    cell_head(nx, ny);
    if (eat) {
        cell_floor(old_food.x, old_food.y);
        food_place();
        hud();
    }
}

static void snake_enter(int arg) {
    (void)arg;
    hi_load();
    state = 0; menu_sel = 0;
    menu_draw();
}

static void snake_exit(void) {}

static void snake_tick(const core_input_t *in, uint32_t delta_ms) {
    if (in->back_pressed) {
        buzzer_click();
        if (state == 0 || state == 2) {
            core_open(core_find("games"), 0);
        } else {
            state = 0;
            menu_draw();
        }
        return;
    }
    if (state == 0) {
        if (in->nav_up && menu_sel > 0) { menu_sel = 0; menu_draw(); buzzer_click(); }
        if (in->nav_down && menu_sel < 1) { menu_sel = 1; menu_draw(); buzzer_click(); }
        if (in->ok_pressed) { buzzer_success(); game_start(menu_sel); }
        return;
    }
    if (state == 2) {
        if (in->ok_pressed) game_start(mode);
        return;
    }
    if (in->nav_left && dir_x != 1)        { dir_x = -1; dir_y = 0; }
    else if (in->nav_right && dir_x != -1) { dir_x = 1;  dir_y = 0; }
    else if (in->nav_up && dir_y != 1)     { dir_y = -1; dir_x = 0; }
    else if (in->nav_down && dir_y != -1)  { dir_y = 1;  dir_x = 0; }

    acc_ms += delta_ms;
    if (acc_ms >= step_ms) {
        acc_ms = 0;
        step();
    }
}

const phoenix_app_t app_snake = {
    .name = "snake",
    .on_enter = snake_enter,
    .on_tick = snake_tick,
    .on_exit = snake_exit,
};
