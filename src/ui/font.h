#ifndef FONT_H
#define FONT_H

#include <SDL2/SDL.h>

#define FONT_SCALE   2
#define FONT_CW      (5 * FONT_SCALE)
#define FONT_CH      (7 * FONT_SCALE)
#define FONT_CSTRIDE (FONT_CW + FONT_SCALE)
#define FONT_LHEIGHT (FONT_CH + FONT_SCALE * 4)

void font_draw_string(SDL_Renderer *r, const char *s, int x, int y, SDL_Color col);
int  font_str_px_w(const char *s);

#endif
