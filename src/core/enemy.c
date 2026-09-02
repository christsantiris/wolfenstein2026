#include "core/enemy.h"
#include "core/difficulty.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ATTACK_AIM_TIME 0.25f
#define ATTACK_FLASH_TIME 0.12f
#define NORMAL_ENEMY_TYPE_COUNT 5

typedef enum {
    ENCOUNTER_INTRO = 0,
    ENCOUNTER_EARLY,
    ENCOUNTER_MIDDLE,
    ENCOUNTER_MIDDLE_DOGS,
    ENCOUNTER_LATE,
    ENCOUNTER_GRENADE_HALLWAYS,
    ENCOUNTER_ELITE,
    ENCOUNTER_KENNELS,
    ENCOUNTER_MINIBOSS,
    ENCOUNTER_BOSS,
    ENCOUNTER_COUNT
} EncounterProfile;

static const EnemyDef ENEMY_DEFS[ENEMY_TYPE_COUNT] = {
    { ENEMY_TYPE_GUARD,          100, 1.8f, 12.0f, 2.0f, 2.0f,  8 },
    { ENEMY_TYPE_OFFICER,         75, 2.4f, 14.0f, 2.0f, 1.5f, 12 },
    { ENEMY_TYPE_SS,             200, 1.4f, 10.0f, 2.0f, 2.5f, 15 },
    { ENEMY_TYPE_BOSS,           850, 1.5f, 18.0f, 2.5f, 1.2f, 26 },
    { ENEMY_TYPE_GUARD_SHOTGUN,  100, 1.8f, 12.0f, 2.0f, 2.0f,  8 },
    { ENEMY_TYPE_MINIBOSS, 700, 1.6f, 16.0f, 7.0f, 1.5f, 18 },
    { ENEMY_TYPE_DOG, 1, 3.8f, 16.0f, 0.85f, 0.65f, 10 },
};

static const EnemyType NORMAL_ENEMY_TYPES[NORMAL_ENEMY_TYPE_COUNT] = {
    ENEMY_TYPE_GUARD,
    ENEMY_TYPE_GUARD_SHOTGUN,
    ENEMY_TYPE_OFFICER,
    ENEMY_TYPE_SS,
    ENEMY_TYPE_DOG
};

static const EncounterProfile LEVEL_ENCOUNTERS[] = {
    ENCOUNTER_COUNT,
    ENCOUNTER_INTRO,
    ENCOUNTER_EARLY,
    ENCOUNTER_EARLY,
    ENCOUNTER_EARLY,
    ENCOUNTER_MIDDLE,
    ENCOUNTER_KENNELS,
    ENCOUNTER_MIDDLE,
    ENCOUNTER_MIDDLE_DOGS,
    ENCOUNTER_MINIBOSS,
    ENCOUNTER_LATE,
    ENCOUNTER_GRENADE_HALLWAYS,
    ENCOUNTER_LATE,
    ENCOUNTER_ELITE,
    ENCOUNTER_ELITE,
    ENCOUNTER_BOSS
};

static const int PROFILE_ROSTERS[ENCOUNTER_COUNT][DIFF_COUNT][NORMAL_ENEMY_TYPE_COUNT] = {
    [ENCOUNTER_INTRO] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 4, 0, 0, 0 },
        [DIFF_DONT_HURT_ME] = { 5, 0, 0, 0 },
        [DIFF_BRING_EM_ON] = { 6, 0, 0, 0 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 6, 1, 0, 0 }
    },
    [ENCOUNTER_EARLY] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 4, 1, 1, 0 },
        [DIFF_DONT_HURT_ME] = { 5, 1, 2, 0 },
        [DIFF_BRING_EM_ON] = { 4, 2, 2, 1 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 4, 2, 2, 2 }
    },
    [ENCOUNTER_MIDDLE] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 3, 1, 3, 1 },
        [DIFF_DONT_HURT_ME] = { 4, 2, 3, 2 },
        [DIFF_BRING_EM_ON] = { 3, 3, 3, 4 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 3, 3, 3, 5 }
    },
    [ENCOUNTER_MIDDLE_DOGS] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 3, 1, 2, 0, 2 },
        [DIFF_DONT_HURT_ME] = { 3, 1, 2, 1, 2 },
        [DIFF_BRING_EM_ON] = { 3, 2, 2, 2, 3 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 3, 2, 3, 3, 4 }
    },
    [ENCOUNTER_LATE] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 3, 2, 3, 2 },
        [DIFF_DONT_HURT_ME] = { 3, 3, 4, 3 },
        [DIFF_BRING_EM_ON] = { 3, 3, 4, 5 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 3, 3, 4, 7 }
    },
    [ENCOUNTER_ELITE] = {
        [DIFF_CAN_I_PLAY_DADDY] = { 2, 2, 3, 4 },
        [DIFF_DONT_HURT_ME] = { 3, 3, 4, 4 },
        [DIFF_BRING_EM_ON] = { 3, 3, 4, 6 },
        [DIFF_I_AM_DEATH_INCARNATE] = { 3, 3, 5, 7 }
    }
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

static int enemy_can_stand(const Map *m, float x, float y, float radius) {
    if (map_is_wall(m, (int)(x - radius), (int)(y - radius))) {
        return 0;
    }
    if (map_is_wall(m, (int)(x + radius), (int)(y - radius))) {
        return 0;
    }
    if (map_is_wall(m, (int)(x - radius), (int)(y + radius))) {
        return 0;
    }
    if (map_is_wall(m, (int)(x + radius), (int)(y + radius))) {
        return 0;
    }
    return 1;
}

int enemy_max_health(EnemyType type, int difficulty) {
    const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
    if (type == ENEMY_TYPE_BOSS) {
        return settings->boss_max_health;
    }
    if (type == ENEMY_TYPE_MINIBOSS) {
        return settings->miniboss_max_health;
    }
    int health = (int)(enemy_def(type)->max_health * settings->enemy_health_multiplier);
    return health > 0 ? health : 1;
}

int enemy_update(Enemy *e, const Player *p, const Map *m, float dt, int difficulty) {
    if (!e->active) {
        return 0;
    }
    const EnemyDef *def = enemy_def(e->type);
    const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
    float speed = def->speed * settings->enemy_speed_multiplier;
    float sight = def->sight_range * settings->enemy_sight_multiplier;
    float attack_cooldown = def->attack_cooldown;

    if (e->type == ENEMY_TYPE_BOSS) {
        int phase_health = enemy_max_health(e->type, difficulty) / 2;
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
    if (e->attack_flash_timer > 0.0f) {
        e->attack_flash_timer -= dt;
        if (e->attack_flash_timer < 0.0f) {
            e->attack_flash_timer = 0.0f;
        }
    }

    if (e->state == ENEMY_IDLE) {
        e->walk_frame = 0;
        e->walk_timer = 0.0f;
        if (dist < sight && enemy_has_los(e, p, m)) {
            e->state = ENEMY_ALERT;
        }
    } else if (e->state == ENEMY_ALERT) {
        e->walk_timer += dt;
        if (e->walk_timer >= 0.25f) {
            e->walk_timer -= 0.25f;
            e->walk_frame = 1 - e->walk_frame;
        }
        if (dist <= def->attack_range) {
            e->state = ENEMY_ATTACK;
            if (e->attack_timer <= 0.0f) {
                e->attack_timer = ATTACK_AIM_TIME;
            }
        } else {
            e->angle = atan2f(dy, dx);
            float nx = e->x + (dx / dist) * speed * dt;
            float ny = e->y + (dy / dist) * speed * dt;
            float collision_radius = 0.28f;
            if (e->type == ENEMY_TYPE_MINIBOSS) {
                collision_radius = 0.48f;
            } else if (e->type == ENEMY_TYPE_DOG) {
                collision_radius = 0.20f;
            }
            if (enemy_can_stand(m, nx, e->y, collision_radius)) {
                e->x = nx;
            }
            if (enemy_can_stand(m, e->x, ny, collision_radius)) {
                e->y = ny;
            }
        }
    } else if (e->state == ENEMY_ATTACK) {
        if (dist > def->attack_range) {
            e->state = ENEMY_ALERT;
        } else if (e->attack_timer <= 0.0f && enemy_has_los(e, p, m)) {
            e->attack_timer = attack_cooldown;
            e->attack_flash_timer = ATTACK_FLASH_TIME;
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

static void place(EnemyList *el, float x, float y, EnemyType type, int difficulty) {
    if (el->count >= MAX_ENEMIES) {
        return;
    }
    Enemy *e = &el->enemies[el->count++];
    e->x = x;
    e->y = y;
    e->angle = (float)(rand() % 8) * ((float)M_PI / 4.0f);
    e->health = enemy_max_health(type, difficulty);
    e->active = 1;
    e->type = type;
    e->state = ENEMY_IDLE;
    e->attack_timer = 0.0f;
    e->attack_flash_timer = 0.0f;
    e->walk_frame = 0;
    e->walk_timer = 0.0f;
    e->reinforcements_called = 0;
}

int enemy_list_call_reinforcements(EnemyList *el, const Player *p, const Map *m, int difficulty) {
    Enemy *leader = NULL;
    for (int i = 0; i < el->count; i++) {
        if (el->enemies[i].type == ENEMY_TYPE_MINIBOSS || el->enemies[i].type == ENEMY_TYPE_BOSS) {
            leader = &el->enemies[i];
            break;
        }
    }
    if (!leader || !leader->active || leader->reinforcements_called) {
        return 0;
    }
    if (leader->health > enemy_max_health(leader->type, difficulty) / 2) {
        return 0;
    }

    leader->reinforcements_called = 1;
    const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
    int d = settings->id;
    static const EnemyType MINIBOSS_WAVE_TYPES[4][5] = {
        { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN },
        { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER },
        { ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_GUARD },
        { ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS, ENEMY_TYPE_GUARD }
    };
    static const EnemyType BOSS_WAVE_TYPES[4][5] = {
        { ENEMY_TYPE_GUARD, ENEMY_TYPE_OFFICER },
        { ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS },
        { ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS },
        { ENEMY_TYPE_OFFICER, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS, ENEMY_TYPE_SS }
    };
    static const float SPAWN_X[8] = { 2.5f, 25.5f, 2.5f, 25.5f, 14.5f, 14.5f, 2.5f, 25.5f };
    static const float SPAWN_Y[8] = { 2.5f, 2.5f, 18.5f, 18.5f, 18.5f, 7.5f, 10.5f, 10.5f };
    int wave_count = leader->type == ENEMY_TYPE_BOSS ? settings->boss_wave_count : settings->miniboss_wave_count;
    const EnemyType *wave_types = leader->type == ENEMY_TYPE_BOSS ? BOSS_WAVE_TYPES[d] : MINIBOSS_WAVE_TYPES[d];

    int spawned = 0;
    for (int i = 0; i < 8 && spawned < wave_count; i++) {
        if (el->count >= MAX_ENEMIES) {
            break;
        }
        float dx = SPAWN_X[i] - p->x;
        float dy = SPAWN_Y[i] - p->y;
        if (dx * dx + dy * dy < 9.0f) {
            continue;
        }
        if (map_cell(m, (int)SPAWN_X[i], (int)SPAWN_Y[i]) != 0) {
            continue;
        }
        place(el, SPAWN_X[i], SPAWN_Y[i], wave_types[spawned], difficulty);
        el->enemies[el->count - 1].state = ENEMY_ALERT;
        spawned++;
    }
    return spawned > 0;
}

int enemy_list_begin_boss_final_stand(EnemyList *el, int difficulty) {
    Enemy *boss = NULL;
    for (int i = 0; i < el->count; i++) {
        Enemy *enemy = &el->enemies[i];
        if (enemy->type == ENEMY_TYPE_BOSS || enemy->type == ENEMY_TYPE_MINIBOSS) {
            boss = enemy;
            continue;
        }
        if (enemy->active) {
            return 0;
        }
    }
    if (!boss || boss->active || boss->reinforcements_called >= 2) {
        return 0;
    }

    boss->health = enemy_max_health(boss->type, difficulty) / 10;
    if (boss->health < 1) {
        boss->health = 1;
    }
    boss->active = 1;
    boss->state = ENEMY_ALERT;
    boss->attack_timer = 0.0f;
    boss->attack_flash_timer = 0.0f;
    boss->reinforcements_called = 2;
    return 1;
}

void enemy_list_init(EnemyList *el, const Map *m, int level, int difficulty, float px, float py) {
    memset(el, 0, sizeof(EnemyList));
    int campaign_level_count = (int)(sizeof(LEVEL_ENCOUNTERS) / sizeof(LEVEL_ENCOUNTERS[0]));
    if (level < 1 || level >= campaign_level_count) {
        return;
    }
    EncounterProfile profile = LEVEL_ENCOUNTERS[level];

    if (profile == ENCOUNTER_MINIBOSS) {
        static const EnemyType SUPPORT_TYPES[DIFF_COUNT][5] = {
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_OFFICER },
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER },
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS },
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS }
        };
        static const float SUPPORT_X[5] = { 5.5f, 22.5f, 5.5f, 21.5f, 22.5f };
        static const float SUPPORT_Y[5] = { 4.5f, 4.5f, 14.5f, 10.5f, 14.5f };
        const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
        int d = settings->id;
        place(el, 14.5f, 4.5f, ENEMY_TYPE_MINIBOSS, difficulty);
        for (int i = 0; i < settings->miniboss_support_count; i++) {
            place(el, SUPPORT_X[i], SUPPORT_Y[i], SUPPORT_TYPES[d][i], difficulty);
        }
        return;
    }

    if (profile == ENCOUNTER_GRENADE_HALLWAYS) {
        static const float ENEMY_X[18] = {
            5.5f, 7.5f, 6.5f, 5.5f, 17.5f, 18.5f, 18.5f, 17.5f, 6.5f,
            8.5f, 7.5f, 6.5f, 18.5f, 19.5f, 19.5f, 18.5f, 7.5f, 20.5f
        };
        static const float ENEMY_Y[18] = {
            2.5f, 7.5f, 15.5f, 19.5f, 2.5f, 7.5f, 15.5f, 19.5f, 3.5f,
            8.5f, 16.5f, 20.5f, 3.5f, 8.5f, 16.5f, 20.5f, 2.5f, 15.5f
        };
        static const EnemyType ENEMY_TYPES[18] = {
            ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD,
            ENEMY_TYPE_OFFICER, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_GUARD_SHOTGUN,
            ENEMY_TYPE_GUARD, ENEMY_TYPE_OFFICER, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER,
            ENEMY_TYPE_SS, ENEMY_TYPE_SS, ENEMY_TYPE_SS, ENEMY_TYPE_SS,
            ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER
        };
        static const int ENEMY_COUNTS[DIFF_COUNT] = { 10, 12, 15, 18 };
        int d = difficulty_get((Difficulty)difficulty)->id;
        for (int i = 0; i < ENEMY_COUNTS[d]; i++) {
            place(el, ENEMY_X[i], ENEMY_Y[i], ENEMY_TYPES[i], difficulty);
        }
        return;
    }

    if (profile == ENCOUNTER_KENNELS) {
        static const float DOG_X[20] = {
            3.5f, 24.5f, 3.5f, 24.5f, 3.5f, 24.5f, 3.5f, 24.5f, 3.5f, 24.5f,
            8.5f, 19.5f, 8.5f, 19.5f, 8.5f, 19.5f, 8.5f, 19.5f, 8.5f, 19.5f
        };
        static const float DOG_Y[20] = {
            1.5f, 1.5f, 5.5f, 5.5f, 9.5f, 9.5f, 14.5f, 14.5f, 18.5f, 18.5f,
            3.5f, 3.5f, 7.5f, 7.5f, 11.5f, 11.5f, 16.5f, 16.5f, 20.5f, 20.5f
        };
        static const int DOG_COUNTS[DIFF_COUNT] = { 11, 14, 16, 18 };
        int d = difficulty_get((Difficulty)difficulty)->id;
        int dog_count = DOG_COUNTS[d];
        for (int i = 0; i < dog_count; i++) {
            place(el, DOG_X[i], DOG_Y[i], ENEMY_TYPE_DOG, difficulty);
        }
        return;
    }

    if (profile == ENCOUNTER_BOSS) {
        static const EnemyType SUPPORT_TYPES[4][6] = {
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS },
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS },
            { ENEMY_TYPE_GUARD, ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS },
            { ENEMY_TYPE_GUARD_SHOTGUN, ENEMY_TYPE_OFFICER, ENEMY_TYPE_OFFICER, ENEMY_TYPE_SS, ENEMY_TYPE_SS, ENEMY_TYPE_SS }
        };
        static const float SUPPORT_X[6] = { 5.5f, 22.5f, 22.5f, 5.5f, 5.5f, 22.5f };
        static const float SUPPORT_Y[6] = { 14.5f, 14.5f, 12.5f, 12.5f, 8.5f, 8.5f };
        const DifficultyDef *settings = difficulty_get((Difficulty)difficulty);
        int d = settings->id;
        place(el, 14.5f, 4.5f, ENEMY_TYPE_BOSS, difficulty);
        for (int i = 0; i < settings->boss_support_count; i++) {
            place(el, SUPPORT_X[i], SUPPORT_Y[i], SUPPORT_TYPES[d][i], difficulty);
        }
        return;
    }

    int d = difficulty_get((Difficulty)difficulty)->id;
    int count = 0;
    for (int type_index = 0; type_index < NORMAL_ENEMY_TYPE_COUNT; type_index++) {
        count += PROFILE_ROSTERS[profile][d][type_index];
    }

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

        int roster_index = i;
        EnemyType type = ENEMY_TYPE_GUARD;
        for (int type_index = 0; type_index < NORMAL_ENEMY_TYPE_COUNT; type_index++) {
            int type_count = PROFILE_ROSTERS[profile][d][type_index];
            if (roster_index < type_count) {
                type = NORMAL_ENEMY_TYPES[type_index];
                break;
            }
            roster_index -= type_count;
        }
        place(el, candidates[i].x, candidates[i].y, type, difficulty);
    }
}
