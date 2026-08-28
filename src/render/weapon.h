#ifndef WEAPON_H
#define WEAPON_H

#include <SDL2/SDL.h>
#include "core/game.h"
#include "render/texture.h"

#define WEAPON_RELOAD_FRAME_COUNT 3

void weapon_render(SDL_Renderer *renderer, const WeaponDef *weapon, const Texture *tex, const Texture reload_tex[WEAPON_RELOAD_FRAME_COUNT], int ammo, float shot_timer, float shot_cooldown, float whip_timer, int is_reloading, float reload_timer, int screen_w, int screen_h);

#endif
