#include "theme_icons.h"
#include "theme.h"
#include "wallpaper.h"
#include "st7789.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

#define ICON_MAGIC     "PHXICN3\n"
#define ICON_MAGIC_LEN 8
#define ICON_FILE_SIZE (ICON_MAGIC_LEN + ICON_CACHE_SIZE)
#define ICON_KEY_HI    0xF8
#define ICON_KEY_LO    0x1F

static uint8_t icon_cache[ICON_CACHE_SIZE];
static bool icon_loaded = false;

/* ---------- Phoenix: ретро-арт ---------- */
static const char *const art_phoenix[5][16] = {
{
"................", ".KKKKKKKKKKKKKK.", ".KGGGGGGGGGGGGK.", ".KGBBBBBBBBBBGK.",
".KGBWWBBBBBBBGK.", ".KGBWWBBBBBBBGK.", ".KGBBBBBBBBBBGK.", ".KGBBWWWWWWBBGK.",
".KGBBBBBBBBBBGK.", ".KGGGGGGGGGGGGK.", ".KKKKKKKKKKKKKK.", "......KDDK......",
"......KDDK......", "....KDDDDDDK....", "...KDDDDDDDDK...", "................"
},
{
"................", ".KKKK...........", ".KyyKKKKKKKKKKK.", ".KYYYYYYYYYYYYK.",
".KYWWWWWWWWWWYK.", ".KYWWWWWWWWWWYK.", ".KYYYYYYYYYYYYK.", ".KYYYYYYYYYYYYK.",
".KYYYYYYYYYYYYK.", ".KYYYYYYYYYYYYK.", ".KyyyyyyyyyyyyK.", ".KKKKKKKKKKKKKK.",
"................", "................", "................", "................"
},
{
"................", ".....KGGGGK.....", ".....KGGGGK.....", "..KKKGGGGGGKKK..",
"..KGGGGGGGGGGK..", ".KGGGGKKKKGGGGK.", ".KGGGK....KGGGK.", ".KGGGK....KGGGK.",
".KGGGK....KGGGK.", ".KGGGK....KGGGK.", ".KGGGGKKKKGGGGK.", "..KGGGGGGGGGGK..",
"..KKKGGGGGGKKK..", ".....KGGGGK.....", ".....KGGGGK.....", "................"
},
{
"................", "..KKKKKKKKKKKK..", ".KDDDDDDDDDDDDK.", ".KDDDWDDDDDRDDK.",
".KDWWWDDDDRRDDK.", ".KDDDWDDDDERDDK.", ".KDDDDDDDDDDDDK.", ".KDDDDDDDDDDDDK.",
".KDDKDDDDDDKDDK.", ".KKKDDDDDDDDKKK.", "..KKKKKKKKKKKK..", "................",
"................", "................", "................", "................"
},
{
"................", ".KKKKKKKKKKKKKK.", ".KWKPKWKPKWKPWK.", ".KKKKKKKKKKKKKK.",
".KPPPPPPPPPPPPK.", ".KPPWPPPPPPPPPK.", ".KPPWWPPPPPPPPK.", ".KPPWWWPPPPPPPK.",
".KPPWWPPPPPPPPK.", ".KPPWPPPPPPPPPK.", ".KPPPPPPPPPPPPK.", ".KKKKKKKKKKKKKK.",
"................", "................", "................", "................"
}
};

/* ---------- Windows XP: стиль Luna ---------- */
static const char *const art_xp[5][16] = {
{
"................",
"..KKKKKKKKKKKK..",
".KNNNNNNNNNNNNK.",
".KNBBBBBBBBBBNK.",
".KNBWWBBBBBBBNK.",
".KNBWWBBBBBBBNK.",
".KNBBBBBBBBBBNK.",
".KNBBBBBBBBBBNK.",
".KNNNNNNNNNNNNK.",
"..KKKKKKKKKKKK..",
"......KGGK......",
"......KGGK......",
"....KGGGGGGK....",
"...KGGGGGGGGK...",
"..KKKKKKKKKKKK..",
"................"
},
{
"................",
"................",
".KKKK...........",
".KYYKKKKKKKKKKK.",
".KYYYYYYYYYYYYK.",
".KYWWYYYYYYYYYK.",
".KYYYYYYYYYYYYK.",
".KYYYYYYYYYYYYK.",
".KYYYYYYYYYYYYK.",
".KyyyyyyyyyyyyK.",
".KKKKKKKKKKKKKK.",
"................",
"................",
"................",
"................",
"................"
},
{
"................",
".....KKKKKK.....",
"....KGGGGGGK....",
"..KKKGGGGGGKKK..",
"..KGGGKBBKGGGK..",
".KGGGKBBBBKGGGK.",
".KGGGKBBBBKGGGK.",
".KGGGKBBBBKGGGK.",
".KGGGKBBBBKGGGK.",
"..KGGGKBBKGGGK..",
"..KKKGGGGGGKKK..",
"....KGGGGGGK....",
".....KKKKKK.....",
"................",
"................",
"................"
},
{
"................",
"................",
"..KKKKKKKKKKKK..",
".KGGGGGGGGGGGGK.",
".KGGWGGGGGRRGGK.",
".KGWWWGGGGRRGGK.",
".KGGWGGGGGYYGGK.",
".KGGGGGGGGGGGGK.",
"..KKKKKKKKKKKK..",
"................",
"................",
"................",
"................",
"................",
"................",
"................"
},
{
"................",
".KKKKKKKKKKKKKK.",
".KBBBBBBBBBBBBK.",
".KBBBEBBBBBBBBK.",
".KBBBEEBBBBBBBK.",
".KBBBEEEBBBBBBK.",
".KBBBEEBBBBBBBK.",
".KBBBEBBBBBBBBK.",
".KBBBBBBBBBBBBK.",
".KGGGGGGGGGGGGK.",
".KKKKKKKKKKKKKK.",
"................",
"................",
"................",
"................",
"................"
}
};

/* ---------- Mac OS Classic: ч/б лайн-арт ---------- */
static const char *const art_macos[5][16] = {
{
"................",
"..KKKKKKKKKKKK..",
"..KWWWWWWWWWWK..",
"..KWKKKKKKKKWK..",
"..KWKWWWWWWKWK..",
"..KWKWWWWWWKWK..",
"..KWKWWWWWWKWK..",
"..KWKWWWWWWKWK..",
"..KWKKKKKKKKWK..",
"..KWWWWWWWWWWK..",
"..KKKKKKKKKKKK..",
"......KKKK......",
"....KKKKKKKK....",
"...KKKKKKKKKK...",
"................",
"................"
},
{
"................",
"................",
".KKKK...........",
".KWWKKKKKKKKKKK.",
".KWWWWWWWWWWWWK.",
".KWWWWWWWWWWWWK.",
".KWWWWWWWWWWWWK.",
".KWWWWWWWWWWWWK.",
".KWWWWWWWWWWWWK.",
".KKKKKKKKKKKKKK.",
"................",
"................",
"................",
"................",
"................",
"................"
},
{
"................",
".KKKKKKKKKKKKKK.",
".KWWWWWWWWWWWWK.",
".KWWWWKWWWWWWWK.",
".KWWWWKWWWWWWWK.",
".KWWWWWWWWKWWWK.",
".KWWWWWWWWKWWWK.",
".KWWKWWWWWWWWWK.",
".KWWKWWWWWWWWWK.",
".KWWWWWWWWWWWWK.",
".KKKKKKKKKKKKKK.",
"................",
"................",
"................",
"................",
"................"
},
{
"................",
"................",
"................",
"......KKKK......",
"......KWWK......",
"......KWWK......",
"......KWWK......",
"...KKKKWWKKKK...",
"..KWWWWWWWWWWK..",
"..KKKKKKKKKKKK..",
"................",
"................",
"................",
"................",
"................",
"................"
},
{
"................",
"....KKKKKKKK....",
"..KWWWWWWWWWWK..",
".KWWWWKKKKWWWWK.",
".KWWWKWWWWKWWWK.",
".KWWWKWWWWKWWWK.",
".KWWWKWWWWKWWWK.",
".KWWWKWWWWKWWWK.",
".KWWWWKKKKWWWWK.",
"..KWWWWWWWWWWK..",
"..KKWWWWWWWWKK..",
"....KKKKKKKK....",
"................",
"................",
"................",
"................"
}
};

static const char *const *const art_sets[THEME_COUNT] = {
    art_phoenix[0], art_xp[0], art_macos[0]
};

static bool file_valid(const char *path) {
    FIL f;
    if (f_open(&f, path, FA_READ) != FR_OK) return false;
    bool ok = false;
    if (f_size(&f) == ICON_FILE_SIZE) {
        char magic[ICON_MAGIC_LEN];
        UINT br = 0;
        if (f_read(&f, magic, ICON_MAGIC_LEN, &br) == FR_OK) {
            ok = (br == ICON_MAGIC_LEN && memcmp(magic, ICON_MAGIC, ICON_MAGIC_LEN) == 0);
        }
    }
    f_close(&f);
    return ok;
}

bool theme_icons_load(int theme_id) {
    icon_loaded = false;
    if (theme_id < 0 || theme_id >= THEME_COUNT) return false;

    static const char *names[THEME_COUNT] = {"phoenix", "xp", "macos"};
    char path[64];
    snprintf(path, sizeof(path), "/themes/%s/icons.rgb", names[theme_id]);

    FIL f;
    if (f_open(&f, path, FA_READ) != FR_OK) return false;
    if (f_size(&f) != ICON_FILE_SIZE) { f_close(&f); return false; }
    char magic[ICON_MAGIC_LEN];
    UINT br = 0;
    f_read(&f, magic, ICON_MAGIC_LEN, &br);
    if (br != ICON_MAGIC_LEN || memcmp(magic, ICON_MAGIC, ICON_MAGIC_LEN) != 0) {
        f_close(&f);
        return false;
    }
    FRESULT fr = f_read(&f, icon_cache, ICON_CACHE_SIZE, &br);
    f_close(&f);
    if (fr != FR_OK || br != ICON_CACHE_SIZE) return false;
    icon_loaded = true;
    return true;
}

bool theme_icons_loaded(void) { return icon_loaded; }

void theme_icons_draw(int icon_id, int x, int y) {
    if (!icon_loaded || icon_id < 0 || icon_id >= ICON_COUNT_INTERNAL) return;
    const theme_t *T = theme_get();
    const uint8_t *data = &icon_cache[(size_t)icon_id * ICON_W * ICON_H * 2];
    static uint8_t row_buf[ICON_W * 2];
    uint8_t dh = (T->desktop >> 8) & 0xFF;
    uint8_t dl = T->desktop & 0xFF;

    for (int r = 0; r < ICON_H; r++) {
        if (wallpaper_is_loaded()) {
            wallpaper_blit_to_buf(row_buf, x, y + r, ICON_W);
        } else {
            for (int p = 0; p < ICON_W; p++) {
                row_buf[2 * p]     = dh;
                row_buf[2 * p + 1] = dl;
            }
        }
        const uint8_t *src = &data[(size_t)r * ICON_W * 2];
        for (int c = 0; c < ICON_W; c++) {
            uint8_t hi = src[c * 2], lo = src[c * 2 + 1];
            if (hi == ICON_KEY_HI && lo == ICON_KEY_LO) continue;
            row_buf[2 * c]     = hi;
            row_buf[2 * c + 1] = lo;
        }
        st7789_write_row(x, y + r, ICON_W, row_buf);
    }
}

static void render_icon_be(int theme_id, int icon_id, uint8_t *dst) {
    const theme_t *T = theme_get();
    static const char *PAL = "KWGDBNYyPRE";
    const char *const *art;
    switch (theme_id) {
        case THEME_WINXP: art = art_xp[icon_id]; break;
        case THEME_MACOS: art = art_macos[icon_id]; break;
        default:          art = art_phoenix[icon_id]; break;
    }
    uint8_t *p = dst;
    for (int r = 0; r < 16; r++) {
        for (int sy = 0; sy < 2; sy++) {
            for (int c = 0; c < 16; c++) {
                const char *pos = strchr(PAL, art[r][c]);
                uint16_t col = pos ? T->icol[pos - PAL] : ((ICON_KEY_HI << 8) | ICON_KEY_LO);
                for (int sx = 0; sx < 2; sx++) {
                    *p++ = (uint8_t)(col >> 8);
                    *p++ = (uint8_t)(col & 0xFF);
                }
            }
        }
    }
}

void theme_icons_provision(void) {
    static const char *names[THEME_COUNT] = {"phoenix", "xp", "macos"};
    static uint8_t buf[ICON_FILE_SIZE];

    for (int t = 0; t < THEME_COUNT; t++) {
        char path[64];
        snprintf(path, sizeof(path), "/themes/%s/icons.rgb", names[t]);
        if (file_valid(path)) continue;

        theme_set(t);
        memcpy(buf, ICON_MAGIC, ICON_MAGIC_LEN);
        for (int i = 0; i < ICON_COUNT_INTERNAL; i++) {
            render_icon_be(t, i, buf + ICON_MAGIC_LEN + (size_t)i * ICON_W * ICON_H * 2);
        }
        FIL f;
        if (f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) continue;
        UINT bw = 0;
        f_write(&f, buf, ICON_FILE_SIZE, &bw);
        f_close(&f);
    }
    (void)art_sets;
}
