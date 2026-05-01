#ifndef GAME_H
#define GAME_H

#include "core/enemy.h"
#include "core/item.h"
#include "core/player.h"

#define AMMO_PICKUP_AMOUNT   8
#define AMMO_RESERVE_MAX    99
#define HEALTH_PICKUP_AMOUNT 25

typedef enum {
    GUN_9MM_HANDGUN = 0,
    GUN_COUNT
} GunType;

typedef struct {
    GunType type;
    const char *sound_path;
    const char *reload_sound_path;
    int max_ammo;
    float shot_cooldown;
    float shot_timer_duration;
    float reload_time;
} WeaponDef;

typedef struct {
    int health;
    int ammo;
    int reserve_ammo;
    int score;
    float shot_timer;
    float shot_cooldown;
    float reload_timer;
    float hit_flash_timer;
    float pistol_whip_timer;
    int is_reloading;
    float level_clear_timer;
    WeaponDef current_weapon;
    EnemyList enemies;
    ItemList items;
} GameState;

const WeaponDef *weapon_def(GunType type);
void game_init(GameState *g);
int  game_shoot(GameState *g, const Player *p);
int  game_pistol_whip(GameState *g, const Player *p);
int  game_reload(GameState *g);
void game_update_timers(GameState *g, float dt);
int  game_update_enemies(GameState *g, const Player *p, const Map *m, float dt);

#endif
