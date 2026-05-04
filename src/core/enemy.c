#include "core/enemy.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

static const EnemyDef ENEMY_DEFS[ENEMY_TYPE_COUNT] = {
    { ENEMY_TYPE_GUARD,   100, 1.8f, 12.0f, 2.0f, 2.0f,  8 },
    { ENEMY_TYPE_OFFICER,  75, 2.4f, 14.0f, 2.0f, 1.5f, 12 },
    { ENEMY_TYPE_SS,      200, 1.4f, 10.0f, 2.0f, 2.5f, 15 },
    { ENEMY_TYPE_BOSS,    850, 1.5f, 18.0f, 2.5f, 1.2f, 26 },
};

const EnemyDef *enemy_def(EnemyType type) {
    if (type < 0 || type >= ENEMY_TYPE_COUNT) {
        return &ENEMY_DEFS[ENEMY_TYPE_GUARD];
    }
    return &ENEMY_DEFS[type];
}

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

static const float SPEED_MULT[4]  = { 0.70f, 1.0f, 1.20f, 1.50f };
static const float SIGHT_MULT[4]  = { 0.70f, 1.0f, 1.30f, 1.60f };
static const float HEALTH_MULT[4] = { 0.6f, 1.0f, 1.2f, 1.3f };

int enemy_update(Enemy *e, const Player *p, const Map *m, float dt, int difficulty) {
    if (!e->active) {
        return 0;
    }
    const EnemyDef *def = enemy_def(e->type);
    int d = difficulty < 4 ? difficulty : 3;
    float speed = def->speed * SPEED_MULT[d];
    float sight = def->sight_range * SIGHT_MULT[d];
    float attack_cooldown = def->attack_cooldown;

    if (e->type == ENEMY_TYPE_BOSS) {
        int phase_health = (int)(def->max_health * HEALTH_MULT[d] * 0.5f);
        if (e->health <= phase_health) {
            speed *= 1.35f;
            attack_cooldown *= 0.65f;
        }
    }

    float dx = p->x - e->x;
    float dy = p->y - e->y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (e->attack_timer > 0.0f) {
        e->attack_timer -= dt;
    }

    if (e->state == ENEMY_IDLE) {
        if (dist < sight && enemy_has_los(e, p, m)) {
            e->state = ENEMY_ALERT;
        }
    } else if (e->state == ENEMY_ALERT) {
        if (dist <= def->attack_range) {
            e->state = ENEMY_ATTACK;
        } else {
            e->angle = atan2f(dy, dx);
            float nx = e->x + (dx / dist) * speed * dt;
            float ny = e->y + (dy / dist) * speed * dt;
            if (!map_is_wall(m, (int)nx, (int)e->y)) {
                e->x = nx;
            }
            if (!map_is_wall(m, (int)e->x, (int)ny)) {
                e->y = ny;
            }
        }
    } else if (e->state == ENEMY_ATTACK) {
        if (dist > def->attack_range) {
            e->state = ENEMY_ALERT;
        } else if (e->attack_timer <= 0.0f && enemy_has_los(e, p, m)) {
            e->attack_timer = attack_cooldown;
            return def->attack_damage;
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

static const int   COUNT_BONUS[4] = { -1,   0,    2,    5   };

static void place(EnemyList *el, float x, float y, EnemyType type, int difficulty) {
    if (el->count >= MAX_ENEMIES) {
        return;
    }
    Enemy *e = &el->enemies[el->count++];
    e->x = x;
    e->y = y;
    e->angle = (float)(rand() % 8) * ((float)M_PI / 4.0f);
    int base_hp = enemy_def(type)->max_health;
    float mult = HEALTH_MULT[difficulty < 4 ? difficulty : 3];
    e->health = (int)(base_hp * mult);
    if (e->health < 1) { e->health = 1; }
    e->active = 1;
    e->type = type;
    e->state = ENEMY_IDLE;
    e->attack_timer = 0.0f;
}

void enemy_list_init(EnemyList *el, const Map *m, int level, int difficulty, float px, float py) {
    memset(el, 0, sizeof(EnemyList));

    if (level == 10) {
        place(el, 14.5f, 4.5f, ENEMY_TYPE_BOSS, difficulty);
        place(el, 5.5f, 14.5f, ENEMY_TYPE_GUARD, difficulty);
        place(el, 22.5f, 14.5f, ENEMY_TYPE_GUARD, difficulty);
        place(el, 22.5f, 12.5f, ENEMY_TYPE_OFFICER, difficulty);
        return;
    }

    int count = 4 + level + COUNT_BONUS[difficulty < 4 ? difficulty : 3];
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

        EnemyType type;
        if (level == 1) {
            type = ENEMY_TYPE_GUARD;
        } else if (level == 2) {
            type = (i < count * 6 / 10) ? ENEMY_TYPE_GUARD : ENEMY_TYPE_OFFICER;
        } else {
            if (i < count / 2) {
                type = ENEMY_TYPE_GUARD;
            } else if (i < count * 3 / 4) {
                type = ENEMY_TYPE_OFFICER;
            } else {
                type = ENEMY_TYPE_SS;
            }
        }
        place(el, candidates[i].x, candidates[i].y, type, difficulty);
    }
}
