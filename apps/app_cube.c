#include "phoenix_core.h"
#include "ui.h"
#include "st7789.h"
#include "buzzer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 160
#define FB_H 120
static uint16_t fb[FB_W * FB_H];
static uint16_t fb_static[FB_W * FB_H];
static uint8_t row_buf[FB_W * 4];

static const float verts[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
};

static const int edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

static float ax = 0.3f, ay = 0.5f, az = 0.2f;
static uint32_t frame_count = 0;

static inline void put(int x, int y, uint16_t c) {
    if (x < 0 || x >= FB_W || y < 0 || y >= FB_H) return;
    fb[y * FB_W + x] = c;
}

static void line(int x0, int y0, int x1, int y1, uint16_t c) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) >> 1;
    int x = x0, y = y0;
    for (int i = 0; i <= (dx > dy ? dx : dy); i++) {
        if (x >= 0 && x < FB_W && y >= 0 && y < FB_H)
            fb[y * FB_W + x] = c;
        if (x == x1 && y == y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x += sx; }
        if (e2 < dy) { err += dx; y += sy; }
    }
}

static void flush(void) {
    for (int y = 0; y < FB_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            uint16_t c = fb[y * FB_W + x] | fb_static[y * FB_W + x];
            row_buf[x * 4] = c >> 8;
            row_buf[x * 4 + 1] = c & 0xFF;
            row_buf[x * 4 + 2] = c >> 8;
            row_buf[x * 4 + 3] = c & 0xFF;
        }
        st7789_write_row(0, y * 2, FB_W * 2, row_buf);
        st7789_write_row(0, y * 2 + 1, FB_W * 2, row_buf);
    }
}

static void enter(int arg) {
    (void)arg;
    ax = 0.3f; ay = 0.5f; az = 0.2f;
    frame_count = 0;
    memset(fb_static, 0, sizeof(fb_static));
    st7789_fill(COLOR_BLACK);
    st7789_draw_string(8, 8, "3D Test - Cube", 0x7F11, COLOR_BLACK, 1);
    st7789_draw_string(8, 220, "BACK: exit", 0x7F11, COLOR_BLACK, 1);
    memcpy(fb_static, fb, sizeof(fb_static));
}

static void tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;
    frame_count++;
    if ((frame_count & 1) == 0) return;
    
    ax += 0.05f; ay += 0.06f; az += 0.03f;
    
    float rot[8][3];
    for (int i = 0; i < 8; i++) {
        float x = verts[i][0], y = verts[i][1], z = verts[i][2];
        float y1 = y * cosf(ax) - z * sinf(ax);
        float z1 = y * sinf(ax) + z * cosf(ax);
        float x2 = x * cosf(ay) + z1 * sinf(ay);
        float z2 = -x * sinf(ay) + z1 * cosf(ay);
        float x3 = x2 * cosf(az) - y1 * sinf(az);
        float y3 = x2 * sinf(az) + y1 * cosf(az);
        rot[i][0] = x3; rot[i][1] = y3; rot[i][2] = z2;
    }
    
    memset(fb, 0, sizeof(fb));
    
    float proj[8][2];
    for (int i = 0; i < 8; i++) {
        float z = rot[i][2] + 3.5f;
        float s = 55.0f / z;
        proj[i][0] = 80 + rot[i][0] * s;
        proj[i][1] = 60 + rot[i][1] * s;
    }
    
    for (int e = 0; e < 12; e++)
        line((int)proj[edges[e][0]][0], (int)proj[edges[e][0]][1],
             (int)proj[edges[e][1]][0], (int)proj[edges[e][1]][1], 0x7F11);
    
    flush();
    
    if (in->back_pressed) { buzzer_click(); core_open(APP_DESKTOP, 0); }
}

const phoenix_app_t app_cube = {
    .name = "cube",
    .on_enter = enter,
    .on_tick = tick,
};
