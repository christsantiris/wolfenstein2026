#ifndef ENEMY_H
#define ENEMY_H

#include "core/map.h"
#include "core/player.h"

#define MAX_ENEMIES 32

typedef enum {
    ENEMY_TYPE_GUARD = 0,
    ENEMY_TYPE_OFFICER,
    ENEMY_TYPE_SS,
    ENEMY_TYPE_COUNT
} EnemyType;

typedef struct {
    EnemyType type;
    int max_health;
    float speed;
    float sight_range;
    float attack_range;
    float attack_cooldown;
    int attack_damage;
} EnemyDef;

typedef enum {
    ENEMY_IDLE,
    ENEMY_ALERT,
    ENEMY_ATTACK
} EnemyState;

typedef struct {
    float x;
    float y;
    float angle;
    int health;
    int active;
    EnemyState state;
    float attack_timer;
    EnemyType type;
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyList;

const EnemyDef *enemy_def(EnemyType type);
void enemy_list_init(EnemyList *el, const Map *m, int level, float px, float py);
int  enemy_update(Enemy *e, const Player *p, const Map *m, float dt);
int  enemy_list_all_dead(const EnemyList *el);

#endif
