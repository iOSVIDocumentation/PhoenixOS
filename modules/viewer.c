#include "viewer.h"
#include "ff.h"
#include "st7789.h"
#include <string.h>

#define VIEWER_BUF_SIZE  16384
#define VIEWER_MAX_LINES 1200
#define VIEWER_COLS      39
#define VIEWER_ROW_H     10

static char vbuf[VIEWER_BUF_SIZE];
static const char *vlines[VIEWER_MAX_LINES];
static int vline_count = 0;
static char vtitle[28];

static bool has_ext(const char *name, const char *ext) {
    int nl = (int)strlen(name);
    int el = (int)strlen(ext);
    if (nl < el + 1) return false;
    if (name[nl - el - 1] != '.') return false;
    for (int i = 0; i < el; i++) {
        char a = name[nl - el + i];
        char b = ext[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (a != b) return false;
    }
    return true;
}

bool viewer_open(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *name = slash ? slash + 1 : path;

    static const char *exts[] = {"txt", "log", "md", "ini", "cfg", "c", "h", "csv"};
    bool ok = false;
    for (int i = 0; i < 8; i++) {
        if (has_ext(name, exts[i])) { ok = true; break; }
    }
    if (!ok) return false;

    FIL f;
    if (f_open(&f, path, FA_READ) != FR_OK) return false;

    UINT br = 0;
    FRESULT fr = f_read(&f, vbuf, VIEWER_BUF_SIZE - 1, &br);
    f_close(&f);
    if (fr != FR_OK) return false;
    vbuf[br] = 0;

    vline_count = 0;
    char *p = vbuf;
    while (*p && vline_count < VIEWER_MAX_LINES) {
        char *eol = strchr(p, '\n');
        if (eol) *eol = 0;
        char *cr = strchr(p, '\r');
        if (cr) *cr = 0;

        int len = (int)strlen(p);
        int off = 0;
        do {
            int chunk = len - off;
            if (chunk > VIEWER_COLS) {
                p[off + VIEWER_COLS] = 0;
                chunk = VIEWER_COLS;
            }
            if (vline_count < VIEWER_MAX_LINES) {
                vlines[vline_count++] = p + off;
            }
            off += chunk;
        } while (off < len);

        if (!eol) break;
        p = eol + 1;
    }

    strncpy(vtitle, name, sizeof(vtitle) - 1);
    vtitle[sizeof(vtitle) - 1] = 0;
    return true;
}

int viewer_get_lines(void) {
    return vline_count;
}

void viewer_draw_header(void) {
    st7789_fill_rect(0, 0, 320, 16, COLOR_WIN_BG);
    st7789_draw_string_fast(4, 4, "Viewer -", COLOR_WHITE, COLOR_WIN_BG, 9);
    st7789_draw_string_fast(76, 4, vtitle, COLOR_WHITE, COLOR_WIN_BG, 30);
    st7789_fill_rect(0, 16, 320, 200, COLOR_WHITE);
}

/* Плавный скролл: каждая строка перезаписывается атомарно, без общей заливки */
void viewer_draw(int first_row) {
    for (int row = 0; row < VIEWER_VISIBLE; row++) {
        int idx = first_row + row;
        const char *line = (idx < vline_count) ? vlines[idx] : "";
        st7789_draw_string_fast(4, 20 + row * VIEWER_ROW_H, line, COLOR_BLACK, COLOR_WHITE, VIEWER_COLS);
    }
    st7789_fill_rect(0, 20 + VIEWER_VISIBLE * VIEWER_ROW_H, 320,
                     216 - (20 + VIEWER_VISIBLE * VIEWER_ROW_H), COLOR_WHITE);
}
