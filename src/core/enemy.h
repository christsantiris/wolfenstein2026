#ifndef ENEMY_H
#define ENEMY_H

#include "core/map.h"
#include "core/player.h"

#define MAX_ENEMIES 32

typedef enum {
    ENEMY_IDLE,
    ENEMY_ALERT,
    ENEMY_ATTACK
} EnemyState;

typedef struct {
    float x;
    float y;
    int health;
    int active;
    EnemyState state;
    float attack_timer;
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyList;

void enemy_list_init(EnemyList *el, const Map *m, int level, float px, float py);
int  enemy_update(Enemy *e, const Player *p, const Map *m, float dt);
int  enemy_list_all_dead(const EnemyList *el);

#endif
