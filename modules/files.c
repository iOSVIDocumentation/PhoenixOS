#include "files.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

static bool fm_entry_after(const fm_entry_t *a, const fm_entry_t *b) {
    if (a->is_dir != b->is_dir) return !a->is_dir;
    return strcmp(a->name, b->name) > 0;
}

static void fm_sort(fm_state_t *st) {
    for (int i = 1; i < st->count; i++) {
        fm_entry_t key = st->items[i];
        int j = i - 1;
        while (j >= 0 && fm_entry_after(&st->items[j], &key)) {
            st->items[j + 1] = st->items[j];
            j--;
        }
        st->items[j + 1] = key;
    }
}

bool fm_open_dir(fm_state_t *st, const char *path) {
    DIR dir;
    FILINFO fno;
    st->count = 0;

    FRESULT fr = f_opendir(&dir, path);
    if (fr != FR_OK) return false;

    while (st->count < FM_MAX_ENTRIES) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (fno.fname[0] == '.') continue;
        if (fno.fattrib & (AM_HID | AM_SYS)) continue;

        fm_entry_t *e = &st->items[st->count];
        strncpy(e->name, fno.fname, FM_NAME_LEN - 1);
        e->name[FM_NAME_LEN - 1] = 0;
        e->is_dir = (fno.fattrib & AM_DIR) != 0;
        e->size = fno.fsize;
        st->count++;
    }
    f_closedir(&dir);

    fm_sort(st);

    strncpy(st->cwd, path, FM_PATH_LEN - 1);
    st->cwd[FM_PATH_LEN - 1] = 0;
    return true;
}

void fm_init(fm_state_t *st) {
    memset(st, 0, sizeof(*st));
    strcpy(st->cwd, "/");
    fm_open_dir(st, st->cwd);
}

bool fm_enter(fm_state_t *st, int idx) {
    if (idx < 0 || idx >= st->count || !st->items[idx].is_dir) return false;
    char newpath[FM_PATH_LEN];
    if (strcmp(st->cwd, "/") == 0) {
        snprintf(newpath, sizeof(newpath), "/%s", st->items[idx].name);
    } else {
        snprintf(newpath, sizeof(newpath), "%s/%s", st->cwd, st->items[idx].name);
    }

    /* Backup current state before attempting to enter */
    fm_state_t backup = *st;

    bool ok = fm_open_dir(st, newpath);
    if (ok) {
        /* Success: save the backup as the parent cache */
        memcpy(st->parent_items, backup.items, sizeof(st->items));
        st->parent_count = backup.count;
        strncpy(st->parent_cwd, backup.cwd, FM_PATH_LEN);
        st->has_parent_cache = true;
    } else {
        /* Failure: restore state to avoid leaving fm_state_t broken */
        *st = backup;
    }
    return ok;
}

bool fm_go_up(fm_state_t *st) {
    if (strcmp(st->cwd, "/") == 0) return false;

    /* Use cache if available to avoid re-reading from SD */
    if (st->has_parent_cache) {
        memcpy(st->items, st->parent_items, sizeof(st->items));
        st->count = st->parent_count;
        strncpy(st->cwd, st->parent_cwd, FM_PATH_LEN);
        st->has_parent_cache = false; /* Cache consumed */
        return true;
    }

    char newpath[FM_PATH_LEN];
    strncpy(newpath, st->cwd, FM_PATH_LEN - 1);
    newpath[FM_PATH_LEN - 1] = 0;
    char *slash = strrchr(newpath, '/');
    if (slash == NULL) return false;
    if (slash == newpath) {
        newpath[1] = 0;
    } else {
        *slash = 0;
    }
    return fm_open_dir(st, newpath);
}
