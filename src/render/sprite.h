#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include "core/player.h"
#include "core/enemy.h"
#include "render/texture.h"

/* Layout: [0..7] walk-A dirs, [8..15] walk-B dirs, [16] attack frame */
#define ENEMY_SPRITE_WALK_B  8
#define ENEMY_SPRITE_ATTACK  16
#define ENEMY_SPRITE_FRAMES  17

void sprite_render_all(SDL_Renderer *renderer, const Player *p, const EnemyList *el, const float *zbuf, const Texture enemy_tex[][ENEMY_SPRITE_FRAMES], int screen_w, int screen_h);

#endif
