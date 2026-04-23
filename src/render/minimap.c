#include "render/minimap.h"
#include <math.h>

#define CELL_SIZE   6
#define MARGIN      8
#define PLAYER_R    3
#define DIR_LEN     8

void minimap_render(SDL_Renderer *renderer, const Map *m, const Player *p) {
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
