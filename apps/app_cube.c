#include "phoenix_core.h"
#include "ui.h"
#include "st7789.h"
#include "buzzer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 320
#define FB_H 240
static uint16_t fb[FB_W * FB_H];
static uint8_t row_buf[FB_W * 2];

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

static inline void line(int x0, int y0, int x1, int y1, uint16_t c) {
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
        memcpy(row_buf, &fb[y * FB_W], FB_W * 2);
        st7789_write_row(0, y, FB_W, row_buf);
    }
}

static void enter(int arg) {
    (void)arg;
    ax = 0.3f; ay = 0.5f; az = 0.2f;
    memset(fb, 0, sizeof(fb));
    st7789_fill(COLOR_BLACK);
}

static void tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;
    ax += 0.06f; ay += 0.07f; az += 0.035f;
    
    memset(fb, 0, sizeof(fb));
    
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
    
    float proj[8][2];
    for (int i = 0; i < 8; i++) {
        float z = rot[i][2] + 3.5f;
        float s = 70.0f / z;
        proj[i][0] = 160 + rot[i][0] * s;
        proj[i][1] = 120 + rot[i][1] * s;
    }
    
    for (int e = 0; e < 12; e++)
        line((int)proj[edges[e][0]][0], (int)proj[edges[e][0]][1],
             (int)proj[edges[e][1]][0], (int)proj[edges[e][1]][1], 0x7F11);
    
    flush();
    
    st7789_draw_string(8, 8, "3D Test - Cube", 0x7F11, COLOR_BLACK, 1);
    st7789_draw_string(8, 220, "BACK: exit", 0x7F11, COLOR_BLACK, 1);
    
    if (in->back_pressed) { buzzer_click(); core_open(APP_DESKTOP, 0); }
}

const phoenix_app_t app_cube = {
    .name = "cube",
    .on_enter = enter,
    .on_tick = tick,
};
