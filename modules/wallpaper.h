#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <stdint.h>
#include <stdbool.h>

#define WP_NAME_LEN 28

int wallpaper_scan(void);
int wallpaper_count(void);
const char *wallpaper_name(int i);
bool wallpaper_draw(const char *path);

bool wallpaper_load(const char *path);
void wallpaper_unload(void);
bool wallpaper_is_loaded(void);
void wallpaper_blit_rect(int x, int y, int w, int h);
void wallpaper_blit_to_buf(uint8_t *dst, int x, int y, int w);

#endif // WALLPAPER_H
