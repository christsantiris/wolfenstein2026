#include "core/enemy.h"
#include <string.h>

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
