#include "core/item.h"
#include <string.h>
#include <stdlib.h>

#define ITEMS_PER_LEVEL 3
#define MIN_SPAWN_DIST2 16.0f

void item_list_init(ItemList *il, const Map *m, int level, float px, float py) {
    memset(il, 0, sizeof(*il));

    if (level == 6) {
        Item *ammo = &il->items[il->count++];
        ammo->x = 8.5f;
        ammo->y = 10.5f;
        ammo->type = ITEM_AMMO;
        ammo->active = 1;

        Item *health = &il->items[il->count++];
        health->x = 14.5f;
        health->y = 17.5f;
        health->type = ITEM_HEALTH;
        health->active = 1;

        Item *reserve = &il->items[il->count++];
        reserve->x = 8.5f;
        reserve->y = 17.5f;
        reserve->type = ITEM_AMMO;
        reserve->active = 1;

        Item *rifle = &il->items[il->count++];
        rifle->x = 18.5f;
        rifle->y = 10.5f;
        rifle->type = ITEM_WEAPON_KIT_BATTLE_RIFLE;
        rifle->active = 1;
        return;
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
            if (dx * dx + dy * dy < MIN_SPAWN_DIST2) { continue; }
            if (nc < 1024) {
                candidates[nc].x = ex;
                candidates[nc].y = ey;
                nc++;
            }
        }
    }

    int count = ITEMS_PER_LEVEL;
    if (count > nc) { count = nc; }

    for (int i = 0; i < count; i++) {
        int j = i + rand() % (nc - i);
        Pos tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;

        Item *it = &il->items[il->count++];
        it->x = candidates[i].x;
        it->y = candidates[i].y;
        it->type = (i % 2 == 0) ? ITEM_AMMO : ITEM_HEALTH;
        it->active = 1;
    }

    if (level == 2 && il->count < MAX_ITEMS && nc > count) {
        int j = count + rand() % (nc - count);
        Item *it = &il->items[il->count++];
        it->x = candidates[j].x;
        it->y = candidates[j].y;
        it->type = ITEM_WEAPON_KIT_DUAL;
        it->active = 1;
    }
    if (level == 4 && il->count < MAX_ITEMS && nc > count) {
        int j = count + rand() % (nc - count);
        Item *it = &il->items[il->count++];
        it->x = candidates[j].x;
        it->y = candidates[j].y;
        it->type = ITEM_WEAPON_KIT;
        it->active = 1;
    }
    if (level == 8 && il->count < MAX_ITEMS) {
        Item *it = &il->items[il->count++];
        it->x = 15.5f;
        it->y = 10.5f;
        it->type = ITEM_WEAPON_KIT_RIFLE_GRENADE;
        it->active = 1;
    }
    if (level == 10 && il->count < MAX_ITEMS && nc > count) {
        int j = count + rand() % (nc - count);
        Item *it = &il->items[il->count++];
        it->x = candidates[j].x;
        it->y = candidates[j].y;
        it->type = ITEM_WEAPON_KIT_AK47;
        it->active = 1;
    }
}
