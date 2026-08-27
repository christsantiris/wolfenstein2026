#ifndef WEAPON_H
#define WEAPON_H

#include <SDL2/SDL.h>
#include "core/game.h"
#include "render/texture.h"

void weapon_render(SDL_Renderer *renderer, const WeaponDef *weapon, const Texture *tex, int ammo, float shot_timer, float shot_cooldown, float whip_timer, int screen_w, int screen_h);

#endif
