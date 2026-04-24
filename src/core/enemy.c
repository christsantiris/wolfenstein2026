#include "core/enemy.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ENEMY_SIGHT_RANGE    12.0f
#define ENEMY_ATTACK_RANGE    2.0f
#define ENEMY_SPEED           1.8f
#define ENEMY_ATTACK_COOLDOWN 2.0f

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
        } else if (e->attack_timer <= 0.0f && enemy_has_los(e, p, m)) {
            e->attack_timer = ENEMY_ATTACK_COOLDOWN;
            return 1;
        }
    }

    return 0;
}

int enemy_list_all_dead(const EnemyList *el) {
    if (el->count == 0) {
        return 0;
    }
    for (int i = 0; i < el->count; i++) {
        if (el->enemies[i].active) {
            return 0;
        }
    }
    return 1;
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

void enemy_list_init(EnemyList *el, const Map *m, int level, float px, float py) {
    memset(el, 0, sizeof(EnemyList));

    int count = 4 + level;
    if (count > MAX_ENEMIES) { count = MAX_ENEMIES; }

    typedef struct { float x; float y; } Pos;
    Pos candidates[1024];
    int nc = 0;

    for (int y = 1; y < m->height - 1; y++) {
        for (int x = 1; x < m->width - 1; x++) {
            if (map_cell(m, x, y) != 0) { continue; }
            float ex = x + 0.5f;
            float ey = y + 0.5f;
            float dx = ex - px;
            float dy = ey - py;
            if (dx * dx + dy * dy < 25.0f) { continue; }
            if (nc < 1024) {
                candidates[nc].x = ex;
                candidates[nc].y = ey;
                nc++;
            }
        }
    }

    for (int i = 0; i < count && i < nc; i++) {
        int j = i + rand() % (nc - i);
        Pos tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
        place(el, candidates[i].x, candidates[i].y);
    }
}
