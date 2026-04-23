#include "core/game.h"
#include <math.h>
#include <stddef.h>

#define SHOT_COOLDOWN  0.5f
#define SHOT_TIMER     0.12f
#define SHOT_DAMAGE    34
#define SHOT_RANGE     20.0f
#define SHOT_CONE      0.15f
#define KILL_SCORE     100

void game_init(GameState *g) {
    g->health       = 100;
    g->ammo         = 8;
    g->score        = 0;
    g->shot_timer   = 0.0f;
    g->shot_cooldown = 0.0f;
    enemy_list_init(&g->enemies);
}

void game_shoot(GameState *g, const Player *p) {
    if (g->ammo <= 0 || g->shot_cooldown > 0.0f) {
        return;
    }
    g->ammo--;
    g->shot_cooldown = SHOT_COOLDOWN;
    g->shot_timer    = SHOT_TIMER;

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

void game_update_timers(GameState *g, float dt) {
    if (g->shot_timer    > 0.0f) { g->shot_timer    -= dt; }
    if (g->shot_cooldown > 0.0f) { g->shot_cooldown -= dt; }
}
