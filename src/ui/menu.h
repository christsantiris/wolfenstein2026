#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

typedef struct {
    int is_open;
} Menu;

void menu_handle_event(Menu *m, const SDL_Event *e, int *running);
void menu_render(SDL_Renderer *r, const Menu *m, int screen_w, int screen_h);

#endif
