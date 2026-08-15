#ifndef FS_MUTEX_H
#define FS_MUTEX_H

#include <stdint.h>

void fs_mutex_init(void);
void fs_mutex_lock(void);
void fs_mutex_unlock(void);

// FatFs callbacks (when FF_FS_REENTRANT=1)
int ff_mutex_create(int vol);
void ff_mutex_delete(int vol);
int ff_mutex_take(int vol, uint32_t timeout_ms);
void ff_mutex_give(int vol);

#endif // FS_MUTEX_H
