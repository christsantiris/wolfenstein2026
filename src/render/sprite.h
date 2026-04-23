#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include "core/player.h"
#include "core/enemy.h"
#include "render/texture.h"

void sprite_render_all(SDL_Renderer *renderer, const Player *p, const EnemyList *el, const float *zbuf, const Texture *sprite_tex, int screen_w, int screen_h);

#endif
