#ifndef INPUT_H
#define INPUT_H

#include "core/player.h"
#include "core/map.h"
#include "core/enemy.h"

void input_update(Player *p, const Map *m, const EnemyList *enemies, float dt);
void input_turn_around(Player *p);

#endif
