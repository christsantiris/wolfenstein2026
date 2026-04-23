#include "core/game.h"
#include <math.h>
#include <stddef.h>

#define SHOT_DAMAGE    34
#define ENEMY_HIT_DMG  15
#define SHOT_RANGE   20.0f
#define SHOT_CONE    0.15f
#define KILL_SCORE   100

static const WeaponDef WEAPON_PISTOL = { 8, 0.5f, 0.12f, 1.5f };

void game_init(GameState *g) {
    g->current_weapon = WEAPON_PISTOL;
    g->health         = 100;
    g->ammo           = g->current_weapon.max_ammo;
    g->score          = 0;
    g->shot_timer     = 0.0f;
    g->shot_cooldown  = 0.0f;
    g->reload_timer    = 0.0f;
    g->hit_flash_timer = 0.0f;
    g->is_reloading    = 0;
    enemy_list_init(&g->enemies);
}

void game_reload(GameState *g) {
    if (g->is_reloading || g->ammo == g->current_weapon.max_ammo) {
        return;
    }
    g->is_reloading = 1;
    g->reload_timer = g->current_weapon.reload_time;
}

void game_shoot(GameState *g, const Player *p) {
    if (g->ammo <= 0 || g->shot_cooldown > 0.0f || g->is_reloading) {
        return;
    }
    g->ammo--;
    /* if (g->auto_reload && g->ammo == 0) { game_reload(g); } */
    g->shot_cooldown = g->current_weapon.shot_cooldown;
    g->shot_timer    = g->current_weapon.shot_timer_duration;

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
        if (fabsf(diff) < SHOT_CONE) {
            best_dist = dist;
            target = e;
        }
    }

    if (target) {
        target->health -= SHOT_DAMAGE;
        if (target->health <= 0) {
            target->active = 0;
            g->score += KILL_SCORE;
        }
    }
}

void game_update_enemies(GameState *g, const Player *p, const Map *m, float dt) {
    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        if (enemy_update(e, p, m, dt)) {
            g->health -= ENEMY_HIT_DMG;
            if (g->health < 0) {
                g->health = 0;
            }
            g->hit_flash_timer = 0.3f;
        }
    }
}

void game_update_timers(GameState *g, float dt) {
    if (g->shot_timer      > 0.0f) { g->shot_timer      -= dt; }
    if (g->shot_cooldown   > 0.0f) { g->shot_cooldown   -= dt; }
    if (g->hit_flash_timer > 0.0f) { g->hit_flash_timer -= dt; }
    if (g->is_reloading) {
        g->reload_timer -= dt;
        if (g->reload_timer <= 0.0f) {
            g->ammo        = g->current_weapon.max_ammo;
            g->reload_timer = 0.0f;
            g->is_reloading = 0;
        }
    }
}
