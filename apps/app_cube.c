#include "phoenix_core.h"
#include "ui.h"
#include "st7789.h"
#include "buzzer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FB_W 320
#define FB_H 240
static uint16_t *fb = NULL;

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
static uint32_t fps = 0;
static uint32_t last_fps_time = 0;

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
        put(x, y, c);
        if (x == x1 && y == y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x += sx; }
        if (e2 < dy) { err += dx; y += sy; }
    }
}

static void fb_draw_char(int x, int y, char ch, uint16_t color) {
    for (int r = 0; r < 8; r++) {
        uint8_t row = st7789_font_row(ch, (uint8_t)r);
        for (int c = 0; c < 8; c++) {
            if (row & (0x80 >> c)) {
                put(x + c, y + r, color);
            }
        }
    }
}

static void fb_draw_string(int x, int y, const char *str, uint16_t color) {
    while (*str) {
        fb_draw_char(x, y, *str++, color);
        x += 8;
    }
}

static void flush(void) {
    if (!fb) return;
    for (int y = 0; y < FB_H; y++) {
        memcpy(row_buf, &fb[y * FB_W], FB_W * 2);
        st7789_write_row(0, y, FB_W, row_buf);
    }
}

static void enter(int arg) {
    (void)arg;
    fb = (uint16_t *)malloc(FB_W * FB_H * sizeof(uint16_t));
    if (!fb) {
        core_open(APP_DESKTOP, 0);
        return;
    }
    ax = 0.3f; ay = 0.5f; az = 0.2f;
    frame_count = 0;
    fps = 0;
    last_fps_time = to_ms_since_boot(get_absolute_time());
    memset(fb, 0, FB_W * FB_H * sizeof(uint16_t));
    st7789_fill(COLOR_BLACK);
}

static void exit_app(void) {
    if (fb) {
        free(fb);
        fb = NULL;
    }
}

static void tick(const core_input_t *in, uint32_t delta_ms) {
    (void)delta_ms;
    if (!fb) return;
    
    ax += 0.06f; ay += 0.07f; az += 0.035f;
    frame_count++;
    
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_fps_time >= 1000) {
        fps = frame_count;
        frame_count = 0;
        last_fps_time = now;
    }
    
    // Полная очистка буфера (быстро, ~0.3 мс)
    memset(fb, 0, FB_W * FB_H * sizeof(uint16_t));
    
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
    
    // Текст в буфер
    fb_draw_string(104, 8, "PhoenixOS 3D", 0x7F11);
    
    char fps_buf[32];
    snprintf(fps_buf, sizeof(fps_buf), "FPS: %lu", (unsigned long)fps);
    fb_draw_string(8, 20, fps_buf, 0x7F11);
    fb_draw_string(8, 220, "BACK: exit", 0x7F11);
    
    flush();
    
    if (in->back_pressed) { buzzer_click(); core_open(APP_DESKTOP, 0); }
}

const phoenix_app_t app_cube = {
    .name = "cube",
    .on_enter = enter,
    .on_tick = tick,
    .on_exit = exit_app,
};
