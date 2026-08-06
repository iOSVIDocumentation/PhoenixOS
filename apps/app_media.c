#include "phoenix_core.h"
#include "ui.h"
#include "buzzer.h"
#include "st7789.h"
#include "ff.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include <string.h>
#include <stdio.h>

/* LCD 24 МГц безопасно на дюпонтах (~20 fps с конвейером).
 * Если стабильно — ставь 48*1000*1000 для ~30 fps. */
#define MV_LCD_HZ (24 * 1000 * 1000)

#define MV_MAX      32
#define MV_NAME_LEN 28
#define MV_PATH_LEN 128
#define MV_W 160
#define MV_H 120
#define MV_ROWS 9
#define FRAME_SZ (MV_W * MV_H * 2)

#define CMD_OPEN 1
#define CMD_READ 2
#define CMD_STOP 3

static char mv_names[MV_MAX][MV_NAME_LEN];
static int mv_count = 0;
static int mv_sel = 0;
static int mv_scroll = 0;

static bool playing = false;
static char play_path[MV_PATH_LEN];

static uint16_t pv_fps;
static uint32_t pv_frames;
static uint32_t pv_frame_idx;
static uint32_t frame_time;
static int cur_slot = 0;

static uint8_t frame_buf[2][FRAME_SZ];
static uint8_t row_buf[LCD_WIDTH * 2];

/* ================= CORE 1: чтение кадров + звук ================= */

static FIL pf;
static uint32_t c1_frames, c1_audio_off, c1_audio_count;
static uint32_t c1_note_idx, c1_note_end;
static bool c1_audio = false;
static bool c1_open = false;

static void core1_worker(void) {
    while (true) {
        if (multicore_fifo_rvalid()) {
            uint32_t cmd = multicore_fifo_pop_blocking();
            if (cmd == CMD_OPEN) {
                const char *path = (const char *)multicore_fifo_pop_blocking();
                uint32_t ok = 0, w = 0, h = 0, fps = 0, audio = 0;
                c1_frames = 0; c1_audio_off = 0; c1_audio_count = 0;
                if (f_open(&pf, path, FA_READ) == FR_OK) {
                    uint8_t hd[24];
                    UINT br = 0;
                    if (f_read(&pf, hd, 24, &br) == FR_OK && br == 24 && memcmp(hd, "PVX1", 4) == 0) {
                        w = hd[4] | (hd[5] << 8);
                        h = hd[6] | (hd[7] << 8);
                        fps = hd[8] | (hd[9] << 8);
                        audio = hd[10] | (hd[11] << 8);
                        c1_frames = (uint32_t)hd[12] | ((uint32_t)hd[13] << 8) | ((uint32_t)hd[14] << 16) | ((uint32_t)hd[15] << 24);
                        c1_audio_off = (uint32_t)hd[16] | ((uint32_t)hd[17] << 8) | ((uint32_t)hd[18] << 16) | ((uint32_t)hd[19] << 24);
                        c1_audio_count = (uint32_t)hd[20] | ((uint32_t)hd[21] << 8) | ((uint32_t)hd[22] << 16) | ((uint32_t)hd[23] << 24);
                        if (w == MV_W && h == MV_H && fps != 0 && c1_frames != 0) {
                            ok = 1;
                            c1_open = true;
                            c1_note_idx = 0;
                            c1_note_end = 0;
                            c1_audio = (audio != 0 && c1_audio_count != 0);
                        } else {
                            f_close(&pf);
                        }
                    } else {
                        f_close(&pf);
                    }
                }
                multicore_fifo_push_blocking(ok);
                multicore_fifo_push_blocking(w | (h << 16));
                multicore_fifo_push_blocking(fps);
                multicore_fifo_push_blocking(c1_frames);
            } else if (cmd == CMD_READ) {
                uint32_t slot = multicore_fifo_pop_blocking();
                uint32_t idx = multicore_fifo_pop_blocking();
                UINT br = 0;
                uint32_t ok = 0;
                if (f_lseek(&pf, 24 + idx * FRAME_SZ) == FR_OK &&
                    f_read(&pf, frame_buf[slot], FRAME_SZ, &br) == FR_OK && br == FRAME_SZ) {
                    ok = 1;
                    }
                    multicore_fifo_push_blocking(ok);
            } else if (cmd == CMD_STOP) {
                if (c1_open) { f_close(&pf); c1_open = false; }
                c1_audio = false;
                buzzer_tone_off();
                multicore_fifo_push_blocking(1);
            }
        } else {
            /* Звуковой насос: чиптюн по нотам */
            if (c1_audio && c1_open && c1_note_idx < c1_audio_count) {
                uint32_t now = to_ms_since_boot(get_absolute_time());
                if (now >= c1_note_end) {
                    uint8_t nb[4];
                    UINT br = 0;
                    if (f_lseek(&pf, c1_audio_off + c1_note_idx * 4) == FR_OK &&
                        f_read(&pf, nb, 4, &br) == FR_OK && br == 4) {
                        uint16_t fq = nb[0] | (nb[1] << 8);
                    uint16_t du = nb[2] | (nb[3] << 8);
                    if (du == 0) du = 100;
                    buzzer_tone_on(fq);
                        c1_note_end = now + du;
                        }
                        c1_note_idx++;
                }
            }
        }
    }
}

void media_core1_init(void) {
    multicore_launch_core1(core1_worker);
}

/* ================= UI списка ================= */

void media_open_path(const char *path) {
    strncpy(play_path, path, MV_PATH_LEN - 1);
    play_path[MV_PATH_LEN - 1] = 0;
}

static bool ends_with_pvx(const char *n) {
    int nl = (int)strlen(n);
    if (nl < 5) return false;
    if (n[nl - 4] != '.') return false;
    for (int i = 0; i < 3; i++) {
        char a = n[nl - 3 + i];
        char b = "pvx"[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (a != b) return false;
    }
    return true;
}

static int mv_scan(void) {
    mv_count = 0;
    DIR dir;
    FILINFO fno;
    if (f_opendir(&dir, "/videos") != FR_OK) return 0;
    while (mv_count < MV_MAX) {
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) break;
        if (fno.fattrib & AM_DIR) continue;
        if (!ends_with_pvx(fno.fname)) continue;
        strncpy(mv_names[mv_count], fno.fname, MV_NAME_LEN - 1);
        mv_names[mv_count][MV_NAME_LEN - 1] = 0;
        mv_count++;
    }
    f_closedir(&dir);
    return mv_count;
}

static void media_draw_row(int idx, int y) {
    uint16_t bg = (idx == mv_sel) ? COLOR_WIN_BG : COLOR_WHITE;
    uint16_t fg = (idx == mv_sel) ? COLOR_WHITE : COLOR_BLACK;
    st7789_fill_rect(3, y, 314, 18, bg);
    st7789_fill_rect(6, y + 3, 10, 12, 0x780F);
    st7789_fill_rect(9, y + 6, 3, 6, COLOR_WHITE);
    st7789_draw_string_fast(22, y + 5, mv_names[idx], fg, bg, 36);
}

static void media_draw_list(void) {
    st7789_fill_rect(0, 0, 320, 216, 0xC618);
    st7789_fill_rect(0, 0, 320, 16, COLOR_WIN_BG);
    st7789_draw_string_fast(4, 4, "Media - /videos", COLOR_WHITE, COLOR_WIN_BG, 39);
    st7789_fill_rect(2, 18, 316, 180, COLOR_WHITE);
    if (mv_count == 0) {
        st7789_draw_string_fast(8, 24, "(no .pvx files)", COLOR_DARK_GRAY, COLOR_WHITE, 37);
        return;
    }
    for (int row = 0; row < MV_ROWS; row++) {
        int idx = mv_scroll + row;
        if (idx >= mv_count) break;
        media_draw_row(idx, 20 + row * 18);
    }
}

/* ================= Воспроизведение ================= */

static void draw_frame(const uint8_t *src_all) {
    for (int r = 0; r < MV_H; r++) {
        const uint8_t *src = &src_all[r * MV_W * 2];
        for (int p = 0; p < MV_W; p++) {
            row_buf[4 * p]     = src[2 * p];
            row_buf[4 * p + 1] = src[2 * p + 1];
            row_buf[4 * p + 2] = src[2 * p];
            row_buf[4 * p + 3] = src[2 * p + 1];
        }
        st7789_write_row(0, 2 * r, LCD_WIDTH, row_buf);
        st7789_write_row(0, 2 * r + 1, LCD_WIDTH, row_buf);
    }
}

static bool media_start(void) {
    multicore_fifo_push_blocking(CMD_OPEN);
    multicore_fifo_push_blocking((uint32_t)play_path);
    uint32_t ok = multicore_fifo_pop_blocking();
    multicore_fifo_pop_blocking(); /* w|h */
    pv_fps = (uint16_t)multicore_fifo_pop_blocking();
    pv_frames = multicore_fifo_pop_blocking();
    if (!ok) return false;

    spi_set_baudrate(spi0, MV_LCD_HZ);
    st7789_display_on();

    /* Предзагрузка кадра 0 */
    multicore_fifo_push_blocking(CMD_READ);
    multicore_fifo_push_blocking(0);
    multicore_fifo_push_blocking(0);
    if (!multicore_fifo_pop_blocking()) {
        multicore_fifo_push_blocking(CMD_STOP);
        multicore_fifo_pop_blocking();
        spi_set_baudrate(spi0, 10 * 1000 * 1000);
        return false;
    }
    cur_slot = 0;
    pv_frame_idx = 1;
    frame_time = to_ms_since_boot(get_absolute_time());
    playing = true;
    st7789_fill(COLOR_BLACK);
    return true;
}

static void media_stop(void) {
    if (playing) {
        multicore_fifo_push_blocking(CMD_STOP);
        multicore_fifo_pop_blocking();
        playing = false;
    }
    spi_set_baudrate(spi0, 10 * 1000 * 1000);
    st7789_display_on();
}

static void media_enter(int arg) {
    media_stop();
    if (arg == 1 && play_path[0]) {
        if (media_start()) return;
        buzzer_error();
    }
    mv_sel = 0;
    mv_scroll = 0;
    mv_scan();
    media_draw_list();
}

static void media_exit(void) {
    media_stop();
}

static void media_tick(const core_input_t *in, uint32_t delta_ms) {
    if (!playing) {
        int old_sel = mv_sel;
        int old_scroll = mv_scroll;

        if (in->nav_up && mv_sel > 0) {
            mv_sel--;
            buzzer_click();
        } else if (in->nav_down && mv_sel < mv_count - 1) {
            mv_sel++;
            buzzer_click();
        }

        if (mv_sel < mv_scroll) mv_scroll = mv_sel;
        if (mv_sel >= mv_scroll + MV_ROWS) mv_scroll = mv_sel - MV_ROWS + 1;

        if (in->ok_pressed && mv_count > 0) {
            snprintf(play_path, MV_PATH_LEN, "/videos/%s", mv_names[mv_sel]);
            if (media_start()) return;
            buzzer_error();
        }

        if (in->back_pressed) {
            buzzer_click();
            core_open(APP_DESKTOP, 0);
            return;
        }

        if (mv_scroll != old_scroll) {
            media_draw_list();
        } else if (mv_sel != old_sel) {
            media_draw_row(old_sel, 20 + (old_sel - mv_scroll) * 18);
            media_draw_row(mv_sel, 20 + (mv_sel - mv_scroll) * 18);
        }
        return;
    }

    /* --- Воспроизведение (конвейер) --- */
    if (in->back_pressed || in->ok_pressed) {
        media_stop();
        mv_scan();
        media_draw_list();
        return;
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t period = 1000 / pv_fps;
    if (now - frame_time < period) return;
    frame_time = now;

    /* Запрашиваем следующий кадр ДО отрисовки текущего (перекрытие) */
    bool requested = false;
    if (pv_frame_idx < pv_frames) {
        multicore_fifo_push_blocking(CMD_READ);
        multicore_fifo_push_blocking(pv_frame_idx & 1);
        multicore_fifo_push_blocking(pv_frame_idx);
        requested = true;
    }

    draw_frame(frame_buf[cur_slot]);

    if (requested) {
        if (!multicore_fifo_pop_blocking()) {
            media_stop();
            mv_scan();
            media_draw_list();
            return;
        }
        cur_slot = pv_frame_idx & 1;
        pv_frame_idx++;
    } else {
        /* дорисовали последний кадр */
        media_stop();
        mv_scan();
        media_draw_list();
    }
}

const phoenix_app_t app_media = {
    .name = "media",
    .on_enter = media_enter,
    .on_tick = media_tick,
    .on_exit = media_exit,
};
