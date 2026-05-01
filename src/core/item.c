#include "core/item.h"
#include <string.h>
#include <stdlib.h>

#define ITEMS_PER_LEVEL 3
#define MIN_SPAWN_DIST2 16.0f

void item_list_init(ItemList *il, const Map *m, int level, float px, float py) {
    memset(il, 0, sizeof(*il));

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
        it->type = ITEM_WEAPON_KIT;
        it->active = 1;
    }
}
