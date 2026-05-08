#ifndef ITEM_RENDER_H
#define ITEM_RENDER_H

#include <SDL2/SDL.h>
#include "core/item.h"
#include "core/player.h"
#include "render/texture.h"

void item_render_all(SDL_Renderer *renderer, const Player *p, const ItemList *il, const float *zbuf, const Texture *ammo_tex, const Texture *health_tex, const Texture *weapon_kit_tex, const Texture *weapon_kit_ak47_tex, const Texture *weapon_kit_dual_tex, int screen_w, int screen_h);

#endif
