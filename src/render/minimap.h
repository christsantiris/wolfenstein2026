#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include "core/map.h"
#include "core/player.h"

void minimap_render(SDL_Renderer *renderer, const Map *m, const Player *p);

#endif
