#include "core/enemy.h"
#include <string.h>
#include <math.h>

#define ENEMY_SIGHT_RANGE    12.0f
#define ENEMY_ATTACK_RANGE    2.0f
#define ENEMY_SPEED           1.8f
#define ENEMY_ATTACK_COOLDOWN 1.2f

static int enemy_has_los(const Enemy *e, const Player *p, const Map *m) {
    float dx = p->x - e->x;
    float dy = p->y - e->y;
    int steps = (int)(sqrtf(dx * dx + dy * dy) / 0.1f) + 1;
    float sx = dx / steps;
    float sy = dy / steps;
    float cx = e->x;
    float cy = e->y;
    for (int i = 0; i < steps; i++) {
        cx += sx;
        cy += sy;
        if (map_is_wall(m, (int)cx, (int)cy)) {
            return 0;
        }
    }
    return 1;
}

int enemy_update(Enemy *e, const Player *p, const Map *m, float dt) {
    if (!e->active) {
        return 0;
    }
    float dx = p->x - e->x;
    float dy = p->y - e->y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (e->attack_timer > 0.0f) {
        e->attack_timer -= dt;
    }

    if (e->state == ENEMY_IDLE) {
        if (dist < ENEMY_SIGHT_RANGE && enemy_has_los(e, p, m)) {
            e->state = ENEMY_ALERT;
        }
    } else if (e->state == ENEMY_ALERT) {
        if (dist <= ENEMY_ATTACK_RANGE) {
            e->state = ENEMY_ATTACK;
        } else {
            float nx = e->x + (dx / dist) * ENEMY_SPEED * dt;
            float ny = e->y + (dy / dist) * ENEMY_SPEED * dt;
            if (!map_is_wall(m, (int)nx, (int)e->y)) {
                e->x = nx;
            }
            if (!map_is_wall(m, (int)e->x, (int)ny)) {
                e->y = ny;
            }
        }
    } else if (e->state == ENEMY_ATTACK) {
        if (dist > ENEMY_ATTACK_RANGE) {
            e->state = ENEMY_ALERT;
        } else if (e->attack_timer <= 0.0f) {
            e->attack_timer = ENEMY_ATTACK_COOLDOWN;
            return 1;
        }
    }

    return 0;
}

static void place(EnemyList *el, float x, float y) {
    if (el->count >= MAX_ENEMIES) {
        return;
    }
    Enemy *e = &el->enemies[el->count++];
    e->x      = x;
    e->y      = y;
    e->health = 100;
    e->active = 1;
}

void enemy_list_init(EnemyList *el) {
    memset(el, 0, sizeof(EnemyList));
    place(el,  5.5f,  5.5f);
    place(el, 22.5f,  5.5f);
    place(el,  5.5f, 14.5f);
    place(el, 22.5f, 14.5f);
    place(el, 14.5f,  3.5f);
}
