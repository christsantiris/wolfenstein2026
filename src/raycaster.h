#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <SDL2/SDL.h>
#include "map.h"
#include "player.h"

void raycaster_render(SDL_Renderer *renderer, const Map *m, const Player *p, int screen_w, int screen_h);

#endif
