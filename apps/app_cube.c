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

static const float cube_verts[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
};

static const int cube_faces[6][4] = {
    {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
    {1, 5, 6, 2}, {3, 2, 6, 7}, {4, 5, 1, 0}
};

static float angle_x = 0.3f, angle_y = 0.5f, angle_z = 0.2f;

static inline void fb_put_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= FB_W || y < 0 || y >= FB_H) return;
    fb[y * FB_W + x] = color;
}

static void fb_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
    if (y0 > y1) { int t; t=x0; x0=x1; x1=t; t=y0; y0=y1; y1=t; }
    if (y0 > y2) { int t; t=x0; x0=x2; x2=t; t=y0; y0=y2; y2=t; }
    if (y1 > y2) { int t; t=x1; x1=x2; x2=t; t=y1; y1=y2; y2=t; }
    int total_height = y2 - y0;
    if (total_height == 0) return;
    for (int y = y0; y <= y2; y++) {
        int second_half = y > y1 || y1 == y0;
        int segment_height = second_half ? y2 - y1 : y1 - y0;
        if (segment_height == 0) continue;
        float alpha = (float)(y - y0) / total_height;
        float beta = second_half ? (float)(y - y1) / segment_height : (float)(y - y0) / segment_height;
        int xa = x0 + (int)((x2 - x0) * alpha);
        int xb = second_half ? x1 + (int)((x2 - x1) * beta) : x0 + (int)((x1 - x0) * beta);
        if (xa > xb) { int t = xa; xa = xb; xb = t; }
        for (int x = xa; x <= xb; x++) fb_put_pixel(x, y, color);
    }
}

static void fb_flush(void) {
    static uint8_t row_buf[FB_W * 2];
    for (int y = 0; y < FB_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            uint16_t c = fb[y * FB_W + x];
            row_buf[x * 2] = c >> 8;
            row_buf[x * 2 + 1] = c & 0xFF;
        }
        st7789_write_row(0, y, FB_W, row_buf);
    }
}

static uint16_t shade_color(uint16_t base, float factor) {
    uint16_t r = (base >> 11) & 0x1F;
    uint16_t g = (base >> 5) & 0x3F;
    uint16_t b = base & 0x1F;
    r = (uint16_t)(r * factor); if (r > 31) r = 31;
    g = (uint16_t)(g * factor); if (g > 63) g = 63;
    b = (uint16_t)(b * factor); if (b > 31) b = 31;
    return (r << 11) | (g << 5) | b;
}

static void cube_enter(int arg) {
    (void)arg;
    angle_x = 0.3f; angle_y = 0.5f; angle_z = 0.2f;
    memset(fb, 0, sizeof(fb));
}

static void cube_tick(const core_input_t *in, uint32_t delta_ms) {
    float dt = delta_ms / 1000.0f;
    angle_x += 1.5f * dt;
    angle_y += 2.0f * dt;
    angle_z += 1.0f * dt;
    
    float rotated[8][3];
    for (int i = 0; i < 8; i++) {
        float x = cube_verts[i][0], y = cube_verts[i][1], z = cube_verts[i][2];
        float y1 = y * cosf(angle_x) - z * sinf(angle_x);
        float z1 = y * sinf(angle_x) + z * cosf(angle_x);
        float x2 = x * cosf(angle_y) + z1 * sinf(angle_y);
        float z2 = -x * sinf(angle_y) + z1 * cosf(angle_y);
        float x3 = x2 * cosf(angle_z) - y1 * sinf(angle_z);
        float y3 = x2 * sinf(angle_z) + y1 * cosf(angle_z);
        rotated[i][0] = x3; rotated[i][1] = y3; rotated[i][2] = z2;
    }
    
    memset(fb, 0, sizeof(fb));
    
    float face_avg_z[6];
    int face_order[6] = {0, 1, 2, 3, 4, 5};
    for (int f = 0; f < 6; f++) {
        float avg_z = 0;
        for (int v = 0; v < 4; v++) avg_z += rotated[cube_faces[f][v]][2];
        face_avg_z[f] = avg_z / 4.0f;
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5 - i; j++) {
            if (face_avg_z[face_order[j]] < face_avg_z[face_order[j+1]]) {
                int t = face_order[j]; face_order[j] = face_order[j+1]; face_order[j+1] = t;
            }
        }
    }
    
    float light_x = 0.0f, light_y = 0.0f, light_z = -1.0f;
    uint16_t base_color = 0x7F11;
    
    for (int fi = 0; fi < 6; fi++) {
        int f = face_order[fi];
        int v0 = cube_faces[f][0], v1 = cube_faces[f][1], v2 = cube_faces[f][2];
        float e1x = rotated[v1][0] - rotated[v0][0];
        float e1y = rotated[v1][1] - rotated[v0][1];
        float e1z = rotated[v1][2] - rotated[v0][2];
        float e2x = rotated[v2][0] - rotated[v0][0];
        float e2y = rotated[v2][1] - rotated[v0][1];
        float e2z = rotated[v2][2] - rotated[v0][2];
        float nx = e1y * e2z - e1z * e2y;
        float ny = e1z * e2x - e1x * e2z;
        float nz = e1x * e2y - e1y * e2x;
        float nlen = sqrtf(nx*nx + ny*ny + nz*nz);
        if (nlen < 0.001f) continue;
        nx /= nlen; ny /= nlen; nz /= nlen;
        if (nz > 0.0f) continue;
        float dot = nx * light_x + ny * light_y + nz * light_z;
        if (dot < 0) dot = 0;
        float brightness = 0.3f + 0.7f * dot;
        uint16_t color = shade_color(base_color, brightness);
        
        float proj[4][2];
        for (int v = 0; v < 4; v++) {
            int vi = cube_faces[f][v];
            float z = rotated[vi][2] + 3.0f;
            float scale = 80.0f / z;
            proj[v][0] = 160 + rotated[vi][0] * scale;
            proj[v][1] = 120 + rotated[vi][1] * scale;
        }
        fb_fill_triangle((int)proj[0][0], (int)proj[0][1],
                         (int)proj[1][0], (int)proj[1][1],
                         (int)proj[2][0], (int)proj[2][1], color);
        fb_fill_triangle((int)proj[0][0], (int)proj[0][1],
                         (int)proj[2][0], (int)proj[2][1],
                         (int)proj[3][0], (int)proj[3][1], color);
    }
    
    fb_flush();
    st7789_draw_string(8, 8, "3D Test - Cube", 0x7F11, COLOR_BLACK, 1);
    st7789_draw_string(8, 220, "BACK: exit", 0x7F11, COLOR_BLACK, 1);
    
    if (in->back_pressed) { buzzer_click(); core_open(APP_DESKTOP, 0); }
}

const phoenix_app_t app_cube = {
    .name = "cube",
    .on_enter = cube_enter,
    .on_tick = cube_tick,
};
