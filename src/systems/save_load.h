#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include "core/game.h"
#include "core/player.h"
#include "core/map.h"

#define SAVE_SLOT_COUNT 3

typedef struct {
    int music_on;
    int sound_on;
    int minimap_on;
    int enemy_positions_on;
} SaveSettings;

int save_game(int slot, int level, const Player *p, const GameState *g, const Map *m, const SaveSettings *s);
int load_game(int slot, int *level, Player *p, GameState *g, Map *m, SaveSettings *s);
int save_slot_exists(int slot);

#endif
