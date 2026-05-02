#include "core/game.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

#define SHOT_RANGE 20.0f
static const int KILL_SCORE[4] = { 75, 100, 150, 200 };
#define WHIP_DAMAGE 50
#define WHIP_RANGE 1.5f
#define WHIP_CONE 0.5f
#define WHIP_DURATION 0.4f

static const WeaponDef WEAPON_PISTOL = { GUN_9MM_HANDGUN, "assets/sounds/handgunshot.mp3", "assets/sounds/handgunreload.mp3", 8, 34, 0.15f, 0.5f, 0.12f, 1.5f };
static const WeaponDef WEAPON_SHOTGUN = { GUN_SHOTGUN, "assets/sounds/shotgun.mp3", "assets/sounds/handgunreload.mp3", 2, 80, 0.30f, 0.8f, 0.15f, 2.0f };
static const WeaponDef WEAPON_AK47 = { GUN_AK47, "assets/sounds/ak47.mp3", "assets/sounds/handgunreload.mp3", 30, 30, 0.25f, 0.12f, 0.08f, 2.0f };

static const WeaponDef *ALL_WEAPONS[GUN_COUNT] = {
    [GUN_9MM_HANDGUN] = &WEAPON_PISTOL,
    [GUN_SHOTGUN] = &WEAPON_SHOTGUN,
    [GUN_AK47] = &WEAPON_AK47,
};

const WeaponDef *weapon_def(GunType type) {
    if (type < 0 || type >= GUN_COUNT) {
        return &WEAPON_PISTOL;
    }
    return ALL_WEAPONS[type];
}

void game_init(GameState *g) {
    memset(g->has_weapon, 0, sizeof(g->has_weapon));
    memset(g->ammo_per_gun, 0, sizeof(g->ammo_per_gun));
    g->has_weapon[GUN_9MM_HANDGUN] = 1;
    g->current_weapon = WEAPON_PISTOL;
    g->health = 100;
    g->ammo = g->current_weapon.max_ammo;
    g->ammo_per_gun[GUN_9MM_HANDGUN] = g->ammo;
    g->reserve_ammo = AMMO_RESERVE_MAX;
    g->score = 0;
    g->shot_timer = 0.0f;
    g->shot_cooldown = 0.0f;
    g->reload_timer = 0.0f;
    g->hit_flash_timer = 0.0f;
    g->pistol_whip_timer = 0.0f;
    g->is_reloading = 0;
    g->level_clear_timer = 0.0f;
    memset(&g->enemies, 0, sizeof(g->enemies));
#ifdef DEBUG_NO_AMMO
    g->ammo = 0;
    g->reserve_ammo = 0;
#endif
#ifdef DEBUG_SHOTGUN
    g->has_weapon[GUN_SHOTGUN] = 1;
    g->ammo_per_gun[GUN_SHOTGUN] = WEAPON_SHOTGUN.max_ammo;
    g->current_weapon = WEAPON_SHOTGUN;
    g->ammo = WEAPON_SHOTGUN.max_ammo;
#endif
}

int game_reload(GameState *g) {
    if (g->is_reloading || g->reserve_ammo == 0 || g->ammo == g->current_weapon.max_ammo) {
        return 0;
    }
    g->is_reloading = 1;
    g->reload_timer = g->current_weapon.reload_time;
    return 1;
}

void game_cycle_weapon(GameState *g) {
    g->ammo_per_gun[g->current_weapon.type] = g->ammo;
    int start = (int)g->current_weapon.type;
    for (int i = 1; i < GUN_COUNT; i++) {
        int next = (start + i) % GUN_COUNT;
        if (g->has_weapon[next]) {
            g->current_weapon = *ALL_WEAPONS[next];
            g->ammo = g->ammo_per_gun[next];
            g->is_reloading = 0;
            g->reload_timer = 0.0f;
            g->shot_cooldown = 0.0f;
            return;
        }
    }
}

int game_shoot(GameState *g, const Player *p) {
    if (g->ammo <= 0 || g->shot_cooldown > 0.0f || g->is_reloading) {
        return 0;
    }
    g->ammo--;
    g->shot_cooldown = g->current_weapon.shot_cooldown;
    g->shot_timer = g->current_weapon.shot_timer_duration;

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
            g->score += KILL_SCORE[g->difficulty < 4 ? g->difficulty : 3];
        }
    }
    return 1;
}

int game_pistol_whip(GameState *g, const Player *p) {
    if (g->ammo > 0 || g->is_reloading || g->pistol_whip_timer > 0.0f) {
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
            g->score += KILL_SCORE[g->difficulty < 4 ? g->difficulty : 3];
        }
    }
    return 1;
}

int game_update_enemies(GameState *g, const Player *p, const Map *m, float dt) {
    int bark = 0;
    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        EnemyState prev = e->state;
        static const float DAMAGE_MULT[4] = { 0.5f, 1.0f, 1.3f, 2.5f };
        int dmg = enemy_update(e, p, m, dt, g->difficulty);
        if (dmg > 0) {
            int d = g->difficulty < 4 ? g->difficulty : 3;
            dmg = (int)(dmg * DAMAGE_MULT[d]);
            if (dmg < 1) { dmg = 1; }
            g->health -= dmg;
            if (g->health < 0) {
                g->health = 0;
            }
            g->hit_flash_timer = 0.3f;
        }
        if (prev == ENEMY_IDLE && e->state == ENEMY_ALERT) {
            bark = 1;
        }
    }
    if (enemy_list_all_dead(&g->enemies) && g->level_clear_timer == 0.0f) {
        g->level_clear_timer = 4.0f;
    }
    return bark;
}

void game_update_timers(GameState *g, float dt) {
    if (g->shot_timer > 0.0f) { g->shot_timer -= dt; }
    if (g->shot_cooldown > 0.0f) { g->shot_cooldown -= dt; }
    if (g->pistol_whip_timer > 0.0f) { g->pistol_whip_timer -= dt; }
    if (g->hit_flash_timer > 0.0f) { g->hit_flash_timer -= dt; }
    if (g->level_clear_timer > 0.0f) { g->level_clear_timer -= dt; }
    if (g->is_reloading) {
        g->reload_timer -= dt;
        if (g->reload_timer <= 0.0f) {
            int needed = g->current_weapon.max_ammo - g->ammo;
            int drawn  = (needed > g->reserve_ammo) ? g->reserve_ammo : needed;
            g->ammo += drawn;
            g->reserve_ammo -= drawn;
            g->reload_timer = 0.0f;
            g->is_reloading = 0;
        }
    }
}
