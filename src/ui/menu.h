#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include "ui/highscore.h"

typedef struct {
    int is_open;
    int music_on;
    int sound_on;
    int difficulty;
    int selected;
} Menu;

typedef enum {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_QUIT,
    MENU_ACTION_NEW_GAME,
    MENU_ACTION_MUSIC_TOGGLE,
    MENU_ACTION_SOUND_TOGGLE,
} MenuAction;

MenuAction menu_handle_event(Menu *m, const SDL_Event *e, int sw, int sh);
void menu_render(SDL_Renderer *r, const Menu *m, int screen_w, int screen_h);

typedef enum {
    GAME_OVER_NONE = 0,
    GAME_OVER_NEW_GAME,
    GAME_OVER_QUIT
} GameOverResult;

void           game_over_render(SDL_Renderer *r, int screen_w, int screen_h, int score, int rank, const HighScoreTable *t);
GameOverResult game_over_handle_event(const SDL_Event *e, int screen_w, int screen_h, int score_count);

#endif
