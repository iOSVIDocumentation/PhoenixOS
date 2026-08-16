#ifndef THEME_ICONS_H
#define THEME_ICONS_H

#include <stdint.h>
#include <stdbool.h>

#define ICON_COUNT_INTERNAL 5
#define ICON_W 32
#define ICON_H 32
#define ICON_CACHE_SIZE (ICON_COUNT_INTERNAL * ICON_W * ICON_H * 2)

bool theme_icons_load(int theme_id);
bool theme_icons_loaded(void);
void theme_icons_draw(int icon_id, int x, int y);
void theme_icons_provision(void);

#endif // THEME_ICONS_H
