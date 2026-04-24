#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <SDL2/SDL.h>
#include "core/map.h"
#include "core/player.h"
#include "render/texture.h"

void raycaster_render(SDL_Renderer *renderer, const Map *m, const Player *p, const Texture *wall_tex, const Texture *door_tex, const Texture *exit_tex, float *zbuf, int screen_w, int screen_h);

#endif
