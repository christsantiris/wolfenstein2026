#ifndef ITEM_H
#define ITEM_H

#include "core/map.h"

#define MAX_ITEMS 32

typedef enum {
    ITEM_AMMO,
    ITEM_HEALTH,
    ITEM_WEAPON_KIT
} ItemType;

typedef struct {
    float x;
    float y;
    ItemType type;
    int active;
} Item;

typedef struct {
    Item items[MAX_ITEMS];
    int count;
} ItemList;

void item_list_init(ItemList *il, const Map *m, int level, float px, float py);

#endif
