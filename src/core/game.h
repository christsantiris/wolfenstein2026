#ifndef GAME_H
#define GAME_H

#include "core/enemy.h"
#include "core/player.h"

typedef struct {
    int max_ammo;
    float shot_cooldown;
    float shot_timer_duration;
    float reload_time;
} WeaponDef;

typedef struct {
    int health;
    int ammo;
    int score;
    float shot_timer;
    float shot_cooldown;
    float reload_timer;
    float hit_flash_timer;
    int is_reloading;
    WeaponDef current_weapon;
    EnemyList enemies;
} GameState;

void game_init(GameState *g);
void game_shoot(GameState *g, const Player *p);
void game_reload(GameState *g);
void game_update_timers(GameState *g, float dt);
void game_update_enemies(GameState *g, const Player *p, const Map *m, float dt);

#endif
