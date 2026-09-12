#include "phoenix_core.h"
#include "ui.h"
#include "st7789.h"
#include "buzzer.h"
#include <math.h>
#include <stdlib.h>

static const float cube_verts[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
};

static const int cube_edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

static float angle_x = 0.3f, angle_y = 0.5f, angle_z = 0.2f;

static void cube_enter(int arg) {
    (void)arg;
    angle_x = 0.3f; angle_y = 0.5f; angle_z = 0.2f;
    st7789_fill(COLOR_BLACK);
}

static void draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        if (x0 >= 0 && x0 < 320 && y0 >= 0 && y0 < 240)
            st7789_fill_rect(x0, y0, 1, 1, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void cube_tick(const core_input_t *in, uint32_t delta_ms) {
    float dt = delta_ms / 1000.0f;
    angle_x += 0.7f * dt;
    angle_y += 1.0f * dt;
    angle_z += 0.5f * dt;
    
    float rotated[8][3];
    for (int i = 0; i < 8; i++) {
        float x = cube_verts[i][0], y = cube_verts[i][1], z = cube_verts[i][2];
        float y1 = y * cosf(angle_x) - z * sinf(angle_x);
        float z1 = y * sinf(angle_x) + z * cosf(angle_x);
        float x2 = x * cosf(angle_y) + z1 * sinf(angle_y);
        float z2 = -x * sinf(angle_y) + z1 * cosf(angle_y);
        float x3 = x2 * cosf(angle_z) - y1 * sinf(angle_z);
        float y3 = x2 * sinf(angle_z) + y1 * cosf(angle_z);
        rotated[i][0] = x3;
        rotated[i][1] = y3;
        rotated[i][2] = z2;
    }
    
    float projected[8][2];
    for (int i = 0; i < 8; i++) {
        float z = rotated[i][2] + 3.0f;
        float scale = 80.0f / z;
        projected[i][0] = 160 + rotated[i][0] * scale;
        projected[i][1] = 120 + rotated[i][1] * scale;
    }
    
    st7789_fill(COLOR_BLACK);
    for (int e = 0; e < 12; e++) {
        int i0 = cube_edges[e][0], i1 = cube_edges[e][1];
        draw_line((int)projected[i0][0], (int)projected[i0][1],
                  (int)projected[i1][0], (int)projected[i1][1], 0x7F11);
    }
    
    st7789_draw_string(8, 8, "3D Test - Cube", 0x7F11, COLOR_BLACK, 1);
    st7789_draw_string(8, 220, "BACK: exit", 0x7F11, COLOR_BLACK, 1);
    
    if (in->back_pressed) {
        buzzer_click();
        core_open(APP_DESKTOP, 0);
    }
}

const phoenix_app_t app_cube = {
    .name = "cube",
    .on_enter = cube_enter,
    .on_tick = cube_tick,
};
