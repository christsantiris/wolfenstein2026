#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include "core/game.h"
#include "render/texture.h"

/* Layout: [0..7] walk-A dirs, [8..15] walk-B dirs, [16] aim, [17] fire, [18] corpse */
#define ENEMY_SPRITE_WALK_B 8
#define ENEMY_SPRITE_AIM 16
#define ENEMY_SPRITE_FIRE 17
#define ENEMY_SPRITE_CORPSE 18
#define ENEMY_SPRITE_FRAMES 19

void sprite_render_all(SDL_Renderer *renderer, const Player *p, const EnemyList *el, float *depth_buffer, const Texture enemy_tex[][ENEMY_SPRITE_FRAMES], int difficulty, int screen_w, int screen_h);
void sprite_render_grenade(SDL_Renderer *renderer, const Player *p, const GrenadeState *grenade, float *depth_buffer, int screen_w, int screen_h);

#endif
