#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>

#define HUD_HEIGHT 70

void hud_render(SDL_Renderer *renderer, int screen_w, int screen_h, int health, int ammo);

#endif
