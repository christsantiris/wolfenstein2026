#ifndef ENEMY_H
#define ENEMY_H

#define MAX_ENEMIES 32

typedef struct {
    float x;
    float y;
    int health;
    int active;
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyList;

void enemy_list_init(EnemyList *el);

#endif
