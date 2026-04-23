#ifndef GAME_H
#define GAME_H

#include "core/enemy.h"

typedef struct {
    int health;
    int ammo;
    int score;
    EnemyList enemies;
} GameState;

void game_init(GameState *g);

#endif
