#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include "core/enemy.h"
#include "core/item.h"
#include "core/map.h"
#include "core/player.h"

void minimap_render(SDL_Renderer *renderer, const Map *m, const Player *p, const EnemyList *enemies, const ItemList *items, int show_enemy_positions, int show_weapon_pickups);

#endif
