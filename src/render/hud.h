#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>
#include "render/texture.h"

#define HUD_HEIGHT 70

typedef enum {
    HUD_FACE_HEALTHY = 0,
    HUD_FACE_WOUNDED,
    HUD_FACE_INJURED,
    HUD_FACE_CRITICAL,
    HUD_FACE_DEAD,
    HUD_FACE_COUNT
} HudFace;

void hud_render(SDL_Renderer *renderer, int screen_w, int screen_h, int health, int ammo, int reserve_ammo, int score, int enemies_remaining, const Texture face_tex[HUD_FACE_COUNT]);
void hud_draw_level_clear(SDL_Renderer *renderer, int screen_w, int screen_h, float timer);
void hud_draw_boss_final_stand(SDL_Renderer *renderer, int screen_w, float timer);
void hud_draw_exit_open(SDL_Renderer *renderer, int screen_w, int screen_h);

#endif
