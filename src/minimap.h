#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include "map.h"
#include "player.h"

void minimap_render(SDL_Renderer *renderer, const Map *m, const Player *p);

#endif
