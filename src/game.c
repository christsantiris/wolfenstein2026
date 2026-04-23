#include "game.h"

void game_init(GameState *g) {
    g->health = 100;
    g->ammo   = 8;
    g->score  = 0;
    enemy_list_init(&g->enemies);
}
