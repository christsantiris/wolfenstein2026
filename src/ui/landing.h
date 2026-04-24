#ifndef LANDING_H
#define LANDING_H

#include <SDL2/SDL.h>
#include "core/difficulty.h"

typedef enum {
    LANDING_NONE = 0,
    LANDING_NEW_GAME,
    LANDING_QUIT
} LandingResult;

void          landing_reset(void);
void          landing_render(SDL_Renderer *r, int sw, int sh);
LandingResult landing_handle_event(const SDL_Event *e, int sw, int sh);

void          difficulty_screen_reset(void);
void          difficulty_screen_render(SDL_Renderer *r, int sw, int sh);
Difficulty    difficulty_screen_handle_event(const SDL_Event *e, int sw, int sh);

#endif
