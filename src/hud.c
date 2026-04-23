#include "hud.h"

#define BAR_PADDING   10
#define BAR_H         20
#define AMMO_BLOCK_W  8
#define AMMO_BLOCK_H  18
#define AMMO_GAP      3
#define FACE_W        50
#define FACE_H        50

void hud_render(SDL_Renderer *renderer, int screen_w, int screen_h, int health, int ammo) {
    int bar_y = screen_h - HUD_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect bg = { 0, bar_y, screen_w, HUD_HEIGHT };
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawLine(renderer, 0, bar_y, screen_w, bar_y);

    int bar_w = screen_w / 3 - BAR_PADDING * 2;
    int bar_x = BAR_PADDING;
    int bar_top = bar_y + (HUD_HEIGHT - BAR_H) / 2;

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_Rect bar_bg = { bar_x, bar_top, bar_w, BAR_H };
    SDL_RenderFillRect(renderer, &bar_bg);

    if (health > 0) {
        int filled = bar_w * health / 100;
        if (health > 60) {
            SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        } else if (health > 30) {
            SDL_SetRenderDrawColor(renderer, 220, 180, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
        }
        SDL_Rect bar_fill = { bar_x, bar_top, filled, BAR_H };
        SDL_RenderFillRect(renderer, &bar_fill);
    }

    int face_x = (screen_w - FACE_W) / 2;
    int face_y = bar_y + (HUD_HEIGHT - FACE_H) / 2;
    SDL_SetRenderDrawColor(renderer, 200, 160, 100, 255);
    SDL_Rect face = { face_x, face_y, FACE_W, FACE_H };
    SDL_RenderFillRect(renderer, &face);

    int ammo_right = screen_w - BAR_PADDING;
    int ammo_top = bar_y + (HUD_HEIGHT - AMMO_BLOCK_H) / 2;
    for (int i = 0; i < ammo && i < 99; i++) {
        int bx = ammo_right - (i + 1) * (AMMO_BLOCK_W + AMMO_GAP);
        if (bx < screen_w / 2 + FACE_W / 2) {
            break;
        }
        SDL_SetRenderDrawColor(renderer, 220, 200, 50, 255);
        SDL_Rect block = { bx, ammo_top, AMMO_BLOCK_W, AMMO_BLOCK_H };
        SDL_RenderFillRect(renderer, &block);
    }
}
