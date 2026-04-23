#ifndef WEAPON_H
#define WEAPON_H

#include <SDL2/SDL.h>
#include "texture.h"

void weapon_render(SDL_Renderer *renderer, const Texture *tex, int screen_w, int screen_h);

#endif
