#include "pico/mutex.h"
#include "pico/stdlib.h"

static mutex_t fs_mutex;
static bool fs_mutex_initialized = false;

void fs_mutex_init(void) {
    if (!fs_mutex_initialized) {
        mutex_init(&fs_mutex);
        fs_mutex_initialized = true;
    }
}

void fs_mutex_lock(void) {
    if (!fs_mutex_initialized) fs_mutex_init();
    mutex_enter_blocking(&fs_mutex);
}

void fs_mutex_unlock(void) {
    if (fs_mutex_initialized) {
        mutex_exit(&fs_mutex);
    }
}

// Обёртки для FatFs (используются внутри fatfs_lib)
int ff_mutex_create(int vol) {
    fs_mutex_init();
    return 1; // success
}

void ff_mutex_delete(int vol) {
    // Pico SDK mutex не требует явного удаления
}

int ff_mutex_take(int vol, uint32_t timeout_ms) {
    if (!fs_mutex_initialized) fs_mutex_init();
    if (timeout_ms == 0xFFFFFFFF) {
        mutex_enter_blocking(&fs_mutex);
        return 1;
    }
    return mutex_enter_timeout_ms(&fs_mutex, timeout_ms);
}

void ff_mutex_give(int vol) {
    if (fs_mutex_initialized) {
        mutex_exit(&fs_mutex);
    }
}
