#ifndef VIEWER_H
#define VIEWER_H

#include <stdbool.h>

#define VIEWER_VISIBLE 19

bool viewer_open(const char *path);
void viewer_draw_header(void);
void viewer_draw(int first_row);
int  viewer_get_lines(void);

#endif // VIEWER_H
