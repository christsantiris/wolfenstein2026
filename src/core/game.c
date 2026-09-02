#include "core/game.h"
#include "core/difficulty.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

#define SHOT_RANGE 20.0f
#define WHIP_DAMAGE 50
#define WHIP_RANGE 1.5f
#define WHIP_CONE 0.5f
#define WHIP_DURATION 0.4f
#define SHOTGUN_PELLET_COUNT 6
#define SHOTGUN_TARGET_RADIUS 0.35f
#define SHOTGUN_UNLOCK_BONUS_MAGAZINES 2
#define GRENADE_SPLASH_RADIUS 2.5f
#define GRENADE_SPLASH_DAMAGE 90
#define GRENADE_SPEED 9.0f
#define GRENADE_MAX_RANGE 20.0f
#define GRENADE_COLLISION_STEP 0.05f
#define GRENADE_ENEMY_RADIUS 0.35f
#define BOSS_FINAL_STAND_ALERT_TIME 3.0f

static const WeaponDef WEAPON_PISTOL = { GUN_9MM_HANDGUN, "assets/sounds/handgunshot.mp3", "assets/sounds/handgunreload.mp3", 8, 99, 34, 0.15f, 0.50f, 0.12f, 1.5f };
static const WeaponDef WEAPON_DUAL_HANDGUN = { GUN_DUAL_HANDGUN, "assets/sounds/handgunshot.mp3", "assets/sounds/handgunreload.mp3", 16, 99, 34, 0.15f, 0.25f, 0.10f, 1.2f };
static const WeaponDef WEAPON_SHOTGUN = { GUN_SHOTGUN, "assets/sounds/shotgun.mp3", "assets/sounds/handgunreload.mp3", 2, 99, 20, 0.30f, 0.80f, 0.15f, 2.0f };
static const WeaponDef WEAPON_AK47 = { GUN_AK47, "assets/sounds/ak-47.mp3", "assets/sounds/handgunreload.mp3", 30, 99, 30, 0.25f, 0.12f, 0.12f, 2.0f };
static const WeaponDef WEAPON_BATTLE_RIFLE = { GUN_BATTLE_RIFLE, "assets/sounds/rifle.mp3", "assets/sounds/handgunreload.mp3", 10, 99, 75, 0.14f, 0.35f, 0.10f, 1.8f };
static const WeaponDef WEAPON_KNIFE = { GUN_KNIFE, "assets/sounds/punch.mp3", NULL, 0, 0, WHIP_DAMAGE, WHIP_CONE, WHIP_DURATION, WHIP_DURATION, 0.0f };
static const WeaponDef WEAPON_RIFLE_GRENADE = { GUN_RIFLE_GRENADE, "assets/sounds/grenade.mp3", "assets/sounds/handgunreload.mp3", 1, 8, 120, 0.20f, 1.10f, 0.18f, 2.4f };
static const WeaponDef WEAPON_UZI = { GUN_UZI, "assets/sounds/ak-47.mp3", "assets/sounds/handgunreload.mp3", 25, 99, 20, 0.20f, 0.14f, 0.10f, 1.7f };

static const WeaponDef *ALL_WEAPONS[GUN_COUNT] = {
    [GUN_9MM_HANDGUN]  = &WEAPON_PISTOL,
    [GUN_DUAL_HANDGUN] = &WEAPON_DUAL_HANDGUN,
    [GUN_SHOTGUN]      = &WEAPON_SHOTGUN,
    [GUN_AK47]         = &WEAPON_AK47,
    [GUN_BATTLE_RIFLE] = &WEAPON_BATTLE_RIFLE,
    [GUN_KNIFE] = &WEAPON_KNIFE,
    [GUN_RIFLE_GRENADE] = &WEAPON_RIFLE_GRENADE,
    [GUN_UZI] = &WEAPON_UZI,
};

static const GunType WEAPON_CYCLE_ORDER[GUN_COUNT] = {
    GUN_KNIFE,
    GUN_9MM_HANDGUN,
    GUN_DUAL_HANDGUN,
    GUN_SHOTGUN,
    GUN_UZI,
    GUN_BATTLE_RIFLE,
    GUN_RIFLE_GRENADE,
    GUN_AK47,
};

const WeaponDef *weapon_def(GunType type) {
    if (type < 0 || type >= GUN_COUNT) {
        return &WEAPON_PISTOL;
    }
    return ALL_WEAPONS[type];
}

int game_ammo_pickup_amount(int difficulty) {
    return difficulty_get((Difficulty)difficulty)->ammo_pickup_amount;
}

int game_weapon_unlock_reserve(const WeaponDef *weapon, int difficulty) {
    if (weapon->type == GUN_RIFLE_GRENADE) {
        return weapon->reserve_capacity;
    }
    const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
    int spare_magazines = settings->weapon_unlock_spare_magazines;
    if (weapon->type == GUN_SHOTGUN) {
        spare_magazines += SHOTGUN_UNLOCK_BONUS_MAGAZINES;
    }
    int reserve = weapon->max_ammo * spare_magazines;
    return reserve < weapon->reserve_capacity ? reserve : weapon->reserve_capacity;
}

int game_level_start_health(int health, int difficulty) {
    int floor = difficulty_get((Difficulty)difficulty)->level_start_health_floor;
    return health < floor ? floor : health;
}

void game_init(GameState *g) {
    memset(g->has_weapon, 0, sizeof(g->has_weapon));
    memset(g->ammo_per_gun, 0, sizeof(g->ammo_per_gun));
    g->has_weapon[GUN_9MM_HANDGUN] = 1;
    g->has_weapon[GUN_KNIFE] = 1;
    g->current_weapon = WEAPON_PISTOL;
    g->health = 100;
    g->ammo = g->current_weapon.max_ammo;
    g->ammo_per_gun[GUN_9MM_HANDGUN] = g->ammo;
    memset(g->reserve_ammo_per_gun, 0, sizeof(g->reserve_ammo_per_gun));
    g->reserve_ammo_per_gun[GUN_9MM_HANDGUN] = WEAPON_PISTOL.reserve_capacity;
    g->score = 0;
    g->shot_timer = 0.0f;
    g->shot_cooldown = 0.0f;
    g->reload_timer = 0.0f;
    g->hit_flash_timer = 0.0f;
    g->pistol_whip_timer = 0.0f;
    g->is_reloading = 0;
    g->level_clear_timer = 0.0f;
    g->boss_final_stand_timer = 0.0f;
    memset(&g->grenade, 0, sizeof(g->grenade));
    memset(&g->enemies, 0, sizeof(g->enemies));
#ifdef DEBUG_NO_AMMO
    g->ammo = 0;
    memset(g->reserve_ammo_per_gun, 0, sizeof(g->reserve_ammo_per_gun));
#endif
#ifdef DEBUG_SHOTGUN
    g->has_weapon[GUN_SHOTGUN] = 1;
    g->ammo_per_gun[GUN_SHOTGUN] = WEAPON_SHOTGUN.max_ammo;
    g->reserve_ammo_per_gun[GUN_SHOTGUN] = WEAPON_SHOTGUN.reserve_capacity;
    g->current_weapon = WEAPON_SHOTGUN;
    g->ammo = WEAPON_SHOTGUN.max_ammo;
#endif
#ifdef DEBUG_AK47
    g->has_weapon[GUN_AK47] = 1;
    g->ammo_per_gun[GUN_AK47] = WEAPON_AK47.max_ammo;
    g->reserve_ammo_per_gun[GUN_AK47] = WEAPON_AK47.reserve_capacity;
    g->current_weapon = WEAPON_AK47;
    g->ammo = WEAPON_AK47.max_ammo;
#endif
#ifdef DEBUG_START_WEAPON
    GunType debug_weapon = (GunType)DEBUG_START_WEAPON;
    if (debug_weapon >= 0 && debug_weapon < GUN_COUNT) {
        const WeaponDef *debug_def = weapon_def(debug_weapon);
        g->has_weapon[debug_weapon] = 1;
        g->ammo_per_gun[debug_weapon] = debug_def->max_ammo;
        if (debug_weapon != GUN_KNIFE) {
            g->reserve_ammo_per_gun[debug_weapon] = debug_def->reserve_capacity;
        }
        g->current_weapon = *debug_def;
        g->ammo = debug_def->max_ammo;
    }
#endif
}

int game_reload(GameState *g) {
    if (g->is_reloading || g->reserve_ammo_per_gun[g->current_weapon.type] == 0 || g->ammo == g->current_weapon.max_ammo) {
        return 0;
    }
    g->is_reloading = 1;
    g->reload_timer = g->current_weapon.reload_time;
    return 1;
}

void game_cycle_weapon(GameState *g) {
    g->ammo_per_gun[g->current_weapon.type] = g->ammo;
    int start = 0;
    for (int i = 0; i < GUN_COUNT; i++) {
        if (WEAPON_CYCLE_ORDER[i] == g->current_weapon.type) {
            start = i;
            break;
        }
    }
    for (int i = 1; i < GUN_COUNT; i++) {
        int next = WEAPON_CYCLE_ORDER[(start + i) % GUN_COUNT];
        if (g->has_weapon[next]) {
            g->current_weapon = *ALL_WEAPONS[next];
            g->ammo = g->ammo_per_gun[next];
            g->is_reloading = 0;
            g->reload_timer = 0.0f;
            g->shot_cooldown = 0.0f;
            g->pistol_whip_timer = 0.0f;
            return;
        }
    }
}

static void game_shoot_shotgun(GameState *g, const Player *p) {
    for (int pellet = 0; pellet < SHOTGUN_PELLET_COUNT; pellet++) {
        float spread = (float)pellet / (float)(SHOTGUN_PELLET_COUNT - 1);
        float pellet_angle = p->angle - g->current_weapon.cone + spread * g->current_weapon.cone * 2.0f;
        float best_dist = SHOT_RANGE;
        Enemy *target = NULL;

        for (int i = 0; i < g->enemies.count; i++) {
            Enemy *e = &g->enemies.enemies[i];
            if (!e->active) {
                continue;
            }
            float dx = e->x - p->x;
            float dy = e->y - p->y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist >= best_dist) {
                continue;
            }
            float angle = atan2f(dy, dx);
            float diff = angle - pellet_angle;
            while (diff > M_PI) {
                diff -= 2.0f * (float)M_PI;
            }
            while (diff < -M_PI) {
                diff += 2.0f * (float)M_PI;
            }
            float target_radius = atan2f(SHOTGUN_TARGET_RADIUS, dist);
            if (fabsf(diff) < target_radius) {
                best_dist = dist;
                target = e;
            }
        }

        if (target) {
            target->health -= g->current_weapon.damage;
            if (target->health <= 0) {
                target->active = 0;
                g->score += difficulty_get((Difficulty)g->difficulty)->kill_score;
            }
        }
    }
}

static int game_path_clear(const Map *map, float x0, float y0, float x1, float y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    int steps = (int)(sqrtf(dx * dx + dy * dy) / 0.1f) + 1;
    for (int step = 1; step < steps; step++) {
        float x = x0 + dx * step / steps;
        float y = y0 + dy * step / steps;
        if (map_is_wall(map, (int)x, (int)y)) {
            return 0;
        }
    }
    return 1;
}

static void game_detonate_grenade(GameState *g, const Map *map, Enemy *target) {
    GrenadeState *grenade = &g->grenade;
    grenade->active = 0;
    grenade->explosion_x = grenade->x;
    grenade->explosion_y = grenade->y;
    grenade->explosion_timer = GRENADE_EXPLOSION_DURATION;
    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *enemy = &g->enemies.enemies[i];
        if (!enemy->active) {
            continue;
        }
        float dx = enemy->x - grenade->x;
        float dy = enemy->y - grenade->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > GRENADE_SPLASH_RADIUS) {
            continue;
        }
        if (enemy != target && !game_path_clear(map, grenade->x, grenade->y, enemy->x, enemy->y)) {
            continue;
        }
        int damage = enemy == target ? g->current_weapon.damage : GRENADE_SPLASH_DAMAGE;
        enemy->health -= damage;
        if (enemy->health <= 0) {
            enemy->active = 0;
            g->score += difficulty_get((Difficulty)g->difficulty)->kill_score;
        }
    }
}

static void game_shoot_rifle_grenade(GameState *g, const Player *p) {
    GrenadeState *grenade = &g->grenade;
    grenade->active = 1;
    grenade->x = p->x;
    grenade->y = p->y;
    grenade->dir_x = cosf(p->angle);
    grenade->dir_y = sinf(p->angle);
    grenade->distance = 0.0f;
    grenade->explosion_timer = 0.0f;
}

int game_shoot(GameState *g, const Player *p, const Map *map) {
    (void)map;
    if (g->current_weapon.type == GUN_KNIFE) {
        return game_pistol_whip(g, p);
    }
    if (g->ammo <= 0 || g->shot_cooldown > 0.0f || g->is_reloading || (g->current_weapon.type == GUN_RIFLE_GRENADE && g->grenade.active)) {
        return 0;
    }
    g->ammo--;
    g->shot_cooldown = g->current_weapon.shot_cooldown;
    g->shot_timer = g->current_weapon.shot_timer_duration;

    if (g->current_weapon.type == GUN_SHOTGUN) {
        game_shoot_shotgun(g, p);
        return 1;
    }
    if (g->current_weapon.type == GUN_RIFLE_GRENADE) {
        game_shoot_rifle_grenade(g, p);
        return 1;
    }

    float best_dist = SHOT_RANGE;
    Enemy *target = NULL;

    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        if (!e->active) {
            continue;
        }
        float dx = e->x - p->x;
        float dy = e->y - p->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist >= best_dist) {
            continue;
        }
        float angle = atan2f(dy, dx);
        float diff = angle - p->angle;
        while (diff >  M_PI) { diff -= 2.0f * (float)M_PI; }
        while (diff < -M_PI) { diff += 2.0f * (float)M_PI; }
        if (fabsf(diff) < g->current_weapon.cone) {
            best_dist = dist;
            target = e;
        }
    }

    if (target) {
        target->health -= g->current_weapon.damage;
        if (target->health <= 0) {
            target->active = 0;
            g->score += difficulty_get((Difficulty)g->difficulty)->kill_score;
        }
    }
    return 1;
}

int game_pistol_whip(GameState *g, const Player *p) {
    int knife_selected = g->current_weapon.type == GUN_KNIFE;
    if ((!knife_selected && g->ammo > 0) || g->is_reloading || g->pistol_whip_timer > 0.0f) {
        return 0;
    }
    g->pistol_whip_timer = WHIP_DURATION;

    float best_dist = WHIP_RANGE;
    Enemy *target = NULL;

    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        if (!e->active) {
            continue;
        }
        float dx = e->x - p->x;
        float dy = e->y - p->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist >= best_dist) {
            continue;
        }
        float angle = atan2f(dy, dx);
        float diff = angle - p->angle;
        while (diff > M_PI) { diff -= 2.0f * (float)M_PI; }
        while (diff < -M_PI) { diff += 2.0f * (float)M_PI; }
        if (fabsf(diff) < WHIP_CONE) {
            best_dist = dist;
            target = e;
        }
    }

    if (target) {
        target->health -= WHIP_DAMAGE;
        if (target->health <= 0) {
            target->active = 0;
            g->score += difficulty_get((Difficulty)g->difficulty)->kill_score;
        }
    }
    return 1;
}

int game_update_grenade(GameState *g, const Map *map, float dt) {
    GrenadeState *grenade = &g->grenade;
    if (grenade->explosion_timer > 0.0f) {
        grenade->explosion_timer -= dt;
        if (grenade->explosion_timer < 0.0f) {
            grenade->explosion_timer = 0.0f;
        }
    }
    if (!grenade->active) {
        return 0;
    }

    float remaining = GRENADE_SPEED * dt;
    while (remaining > 0.0f) {
        float step = remaining < GRENADE_COLLISION_STEP ? remaining : GRENADE_COLLISION_STEP;
        float next_x = grenade->x + grenade->dir_x * step;
        float next_y = grenade->y + grenade->dir_y * step;
        if (map_is_wall(map, (int)next_x, (int)next_y)) {
            game_detonate_grenade(g, map, NULL);
            return 1;
        }

        Enemy *target = NULL;
        for (int i = 0; i < g->enemies.count; i++) {
            Enemy *enemy = &g->enemies.enemies[i];
            if (!enemy->active) {
                continue;
            }
            float dx = enemy->x - next_x;
            float dy = enemy->y - next_y;
            float radius = GRENADE_ENEMY_RADIUS;
            if (enemy->type == ENEMY_TYPE_BOSS || enemy->type == ENEMY_TYPE_MINIBOSS) {
                radius = 0.55f;
            }
            if (dx * dx + dy * dy <= radius * radius) {
                target = enemy;
                break;
            }
        }

        grenade->x = next_x;
        grenade->y = next_y;
        grenade->distance += step;
        remaining -= step;
        if (target || grenade->distance >= GRENADE_MAX_RANGE) {
            game_detonate_grenade(g, map, target);
            return 1;
        }
    }
    return 0;
}

int game_update_enemies(GameState *g, const Player *p, const Map *m, float dt) {
    int sound_events = GAME_ENEMY_SOUND_NONE;
    const DifficultyDef *settings = difficulty_get((Difficulty)g->difficulty);
    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        EnemyState prev = e->state;
        int dmg = enemy_update(e, p, m, dt, g->difficulty);
        if (dmg > 0) {
            dmg = (int)(dmg * settings->enemy_damage_multiplier);
            if (dmg < 1) { dmg = 1; }
            g->health -= dmg;
            if (g->health < 0) {
                g->health = 0;
            }
            g->hit_flash_timer = 0.3f;
            if (e->type == ENEMY_TYPE_DOG) {
                sound_events |= GAME_ENEMY_SOUND_DOG;
            }
        }
        if (prev == ENEMY_IDLE && e->state == ENEMY_ALERT) {
            if (e->type == ENEMY_TYPE_DOG) {
                sound_events |= GAME_ENEMY_SOUND_DOG;
            } else {
                sound_events |= GAME_ENEMY_SOUND_GENERIC;
            }
        }
    }
    if (enemy_list_call_reinforcements(&g->enemies, p, m, g->difficulty)) {
        sound_events |= GAME_ENEMY_SOUND_GENERIC;
    }
    if (enemy_list_begin_boss_final_stand(&g->enemies, g->difficulty)) {
        g->boss_final_stand_timer = BOSS_FINAL_STAND_ALERT_TIME;
        sound_events |= GAME_ENEMY_SOUND_GENERIC;
    }
    if (enemy_list_all_dead(&g->enemies) && g->level_clear_timer == 0.0f) {
        g->level_clear_timer = 4.0f;
    }
    return sound_events;
}

void game_update_timers(GameState *g, float dt) {
    if (g->shot_timer > 0.0f) { g->shot_timer -= dt; }
    if (g->shot_cooldown > 0.0f) { g->shot_cooldown -= dt; }
    if (g->pistol_whip_timer > 0.0f) { g->pistol_whip_timer -= dt; }
    if (g->hit_flash_timer > 0.0f) { g->hit_flash_timer -= dt; }
    if (g->level_clear_timer > 0.0f) { g->level_clear_timer -= dt; }
    if (g->boss_final_stand_timer > 0.0f) { g->boss_final_stand_timer -= dt; }
    if (g->is_reloading) {
        g->reload_timer -= dt;
        if (g->reload_timer <= 0.0f) {
            int needed = g->current_weapon.max_ammo - g->ammo;
            int reserve = g->reserve_ammo_per_gun[g->current_weapon.type];
            int drawn = (needed > reserve) ? reserve : needed;
            g->ammo += drawn;
            g->reserve_ammo_per_gun[g->current_weapon.type] -= drawn;
            g->reload_timer = 0.0f;
            g->is_reloading = 0;
        }
    }
}
