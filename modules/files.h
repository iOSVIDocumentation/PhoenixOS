#ifndef FILES_H
#define FILES_H

#include <stdint.h>
#include <stdbool.h>

#define FM_MAX_ENTRIES 64
#define FM_NAME_LEN    28
#define FM_PATH_LEN    128

typedef struct {
    char name[FM_NAME_LEN];
    bool is_dir;
    uint32_t size;
} fm_entry_t;

typedef struct {
    fm_entry_t items[FM_MAX_ENTRIES];
    int count;
    char cwd[FM_PATH_LEN];
} fm_state_t;

void fm_init(fm_state_t *st);
bool fm_open_dir(fm_state_t *st, const char *path);
bool fm_enter(fm_state_t *st, int idx);
bool fm_go_up(fm_state_t *st);

#endif // FILES_H
