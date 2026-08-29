#include "render/minimap.h"
#include <math.h>

#define CELL_SIZE   6
#define MARGIN      8
#define PLAYER_R    3
#define DIR_LEN     8
#define ENEMY_SIZE  4
#define WEAPON_SIZE 4

static int minimap_is_weapon_pickup(ItemType type) {
    return type == ITEM_WEAPON_KIT || type == ITEM_WEAPON_KIT_AK47 || type == ITEM_WEAPON_KIT_DUAL || type == ITEM_WEAPON_KIT_BATTLE_RIFLE || type == ITEM_WEAPON_KIT_RIFLE_GRENADE;
}

void minimap_render(SDL_Renderer *renderer, const Map *m, const Player *p, const EnemyList *enemies, const ItemList *items, int show_enemy_positions, int show_weapon_pickups) {
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            SDL_Rect cell = {
                MARGIN + x * CELL_SIZE,
                MARGIN + y * CELL_SIZE,
                CELL_SIZE - 1,
                CELL_SIZE - 1
            };
            if (map_is_wall(m, x, y)) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 180);
            } else {
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 180);
            }
            SDL_RenderFillRect(renderer, &cell);
        }
    }

    if (show_enemy_positions) {
        SDL_SetRenderDrawColor(renderer, 255, 105, 30, 255);
        for (int i = 0; i < enemies->count; i++) {
            const Enemy *enemy = &enemies->enemies[i];
            if (!enemy->active) {
                continue;
            }
            int ex = MARGIN + (int)(enemy->x * CELL_SIZE);
            int ey = MARGIN + (int)(enemy->y * CELL_SIZE);
            SDL_Rect marker = { ex - ENEMY_SIZE / 2, ey - ENEMY_SIZE / 2, ENEMY_SIZE, ENEMY_SIZE };
            SDL_RenderFillRect(renderer, &marker);
        }
    }

    if (show_weapon_pickups) {
        SDL_SetRenderDrawColor(renderer, 40, 210, 255, 255);
        for (int i = 0; i < items->count; i++) {
            const Item *item = &items->items[i];
            if (!item->active || !minimap_is_weapon_pickup(item->type)) {
                continue;
            }
            int item_x = MARGIN + (int)(item->x * CELL_SIZE);
            int item_y = MARGIN + (int)(item->y * CELL_SIZE);
            SDL_Rect marker = { item_x - WEAPON_SIZE / 2, item_y - WEAPON_SIZE / 2, WEAPON_SIZE, WEAPON_SIZE };
            SDL_RenderFillRect(renderer, &marker);
        }
    }

    int px = MARGIN + (int)(p->x * CELL_SIZE);
    int py = MARGIN + (int)(p->y * CELL_SIZE);

    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    SDL_Rect dot = { px - PLAYER_R, py - PLAYER_R, PLAYER_R * 2, PLAYER_R * 2 };
    SDL_RenderFillRect(renderer, &dot);

    int dx = (int)(cosf(p->angle) * DIR_LEN);
    int dy = (int)(sinf(p->angle) * DIR_LEN);
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    SDL_RenderDrawLine(renderer, px, py, px + dx, py + dy);
}
