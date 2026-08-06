#include "phoenix_core.h"
#include "st7789.h"
#include "sound.h"
#include <math.h>

#define MAP_W 16
#define MAP_H 16
static const int map[MAP_W][MAP_H] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,1,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,1,1,1,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,1,1,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,1,1,1,1,1,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static float px = 3.5f, py = 3.5f, pa = 0.0f;

static void wolf_enter(int arg) {
    px = 3.5f; py = 3.5f; pa = 0.0f;
    snd_init();
    st7789_fill(COLOR_BLACK);
}

static void wolf_tick(const core_input_t *in, uint32_t delta_ms) {
    // ИСПРАВЛЕНО: убрана опечатка 64
    if (in->back_pressed || in->start_pressed) {
        snd_click();
        core_open(APP_DESKTOP, 0);
        return;
    }

    // ПРОФЕССИОНАЛЬНЫЙ ПОДХОД: скорость масштабируется от delta_ms
    // 0.15f - базовая скорость за 16мс (60 FPS). 
    float speed_mult = delta_ms / 16.0f;
    float move_speed = 0.15f * speed_mult;
    float rot_speed = 0.08f * speed_mult;

    if (in->nav_up) {
        float nx = px + cosf(pa) * move_speed;
        float ny = py + sinf(pa) * move_speed;
        if (map[(int)nx][(int)py] == 0) px = nx;
        if (map[(int)px][(int)ny] == 0) py = ny;
    }
    if (in->nav_down) {
        float nx = px - cosf(pa) * move_speed;
        float ny = py - sinf(pa) * move_speed;
        if (map[(int)nx][(int)py] == 0) px = nx;
        if (map[(int)px][(int)ny] == 0) py = ny;
    }
    if (in->nav_left) pa -= rot_speed;
    if (in->nav_right) pa += rot_speed;
    
    if (in->ok_pressed) {
        snd_shoot(); // Теперь здесь настоящий звук выстрела!
        st7789_fill(COLOR_WHITE);
        sleep_ms(50);
        return; 
    }

    st7789_fill_rect(0, 0, 320, 120, COLOR_BLACK);
    st7789_fill_rect(0, 120, 320, 120, COLOR_DARK_GRAY);

    for (int x = 0; x < 320; x++) {
        float ray_angle = (pa - 3.14159f / 6.0f) + (x / 320.0f) * (3.14159f / 3.0f);
        float sin_a = sinf(ray_angle);
        float cos_a = cosf(ray_angle);
        float dist = 0.0f;
        int hit = 0;
        float test_x = px, test_y = py;

        while (!hit && dist < 20.0f) {
            dist += 0.1f;
            test_x = px + cos_a * dist;
            test_y = py + sin_a * dist;
            if (test_x < 0 || test_x >= MAP_W || test_y < 0 || test_y >= MAP_H) {
                hit = 1; dist = 20.0f;
            } else if (map[(int)test_x][(int)test_y] == 1) {
                hit = 1;
            }
        }

        float corrected_dist = dist * cosf(ray_angle - pa);
        int wall_height = (int)(240.0f / (corrected_dist + 0.001f));
        if (wall_height > 240) wall_height = 240;
        if (wall_height < 1) wall_height = 1;

        int ceiling = (240 - wall_height) / 2;
        uint16_t wall_color = (dist < 4.0f) ? COLOR_LIGHT_BLUE : ((dist < 8.0f) ? COLOR_BLUE : COLOR_DARK_GRAY);

        st7789_fill_rect(x, ceiling, 1, wall_height, wall_color);
    }
}

const phoenix_app_t app_wolf3d = {
    .name = "Wolf3D",
    .on_enter = wolf_enter,
    .on_tick = wolf_tick,
    .on_exit = NULL
};
