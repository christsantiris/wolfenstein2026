#ifndef GAME_H
#define GAME_H

#include "core/enemy.h"
#include "core/item.h"
#include "core/player.h"

typedef enum {
    GUN_9MM_HANDGUN = 0,
    GUN_DUAL_HANDGUN,
    GUN_SHOTGUN,
    GUN_AK47,
    GUN_BATTLE_RIFLE,
    GUN_KNIFE,
    GUN_RIFLE_GRENADE,
    GUN_COUNT
} GunType;

typedef struct {
    GunType type;
    const char *sound_path;
    const char *reload_sound_path;
    int max_ammo;
    int reserve_capacity;
    int damage;
    float cone;
    float shot_cooldown;
    float shot_timer_duration;
    float reload_time;
} WeaponDef;

#define GRENADE_EXPLOSION_DURATION 0.45f

typedef struct {
    int active;
    float x;
    float y;
    float dir_x;
    float dir_y;
    float distance;
    float explosion_x;
    float explosion_y;
    float explosion_timer;
} GrenadeState;

typedef enum {
    GAME_ENEMY_SOUND_NONE = 0,
    GAME_ENEMY_SOUND_GENERIC = 1,
    GAME_ENEMY_SOUND_DOG = 2
} GameEnemySound;

typedef struct {
    int difficulty;
    int health;
    int ammo;
    int reserve_ammo_per_gun[GUN_COUNT];
    int score;
    float shot_timer;
    float shot_cooldown;
    float reload_timer;
    float hit_flash_timer;
    float pistol_whip_timer;
    int is_reloading;
    float level_clear_timer;
    float boss_final_stand_timer;
    WeaponDef current_weapon;
    int has_weapon[GUN_COUNT];
    int ammo_per_gun[GUN_COUNT];
    EnemyList enemies;
    ItemList items;
    GrenadeState grenade;
} GameState;

const WeaponDef *weapon_def(GunType type);
int game_ammo_pickup_amount(int difficulty);
int game_weapon_unlock_reserve(const WeaponDef *weapon, int difficulty);
int game_level_start_health(int health, int difficulty);
void game_init(GameState *g);
int  game_shoot(GameState *g, const Player *p, const Map *m);
int  game_pistol_whip(GameState *g, const Player *p);
int  game_reload(GameState *g);
void game_cycle_weapon(GameState *g);
void game_update_timers(GameState *g, float dt);
int game_update_grenade(GameState *g, const Map *m, float dt);
int game_update_enemies(GameState *g, const Player *p, const Map *m, float dt);

#endif
