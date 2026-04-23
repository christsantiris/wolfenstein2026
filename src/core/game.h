#ifndef GAME_H
#define GAME_H

#include "core/enemy.h"
#include "core/player.h"

typedef struct {
    int health;
    int ammo;
    int score;
    float shot_timer;
    float shot_cooldown;
    EnemyList enemies;
} GameState;

void game_init(GameState *g);
void game_shoot(GameState *g, const Player *p);
void game_update_timers(GameState *g, float dt);

#endif
