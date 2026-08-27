#include "render/hud.h"
#include "ui/font.h"
#include <math.h>
#include <stdio.h>

#define BAR_PADDING   10
#define BAR_H         20
#define AMMO_BLOCK_W  8
#define AMMO_BLOCK_H  18
#define AMMO_GAP      3
#define FACE_W        50
#define FACE_H        50

static HudFace hud_face_for_health(int health) {
    if (health <= 0) {
        return HUD_FACE_DEAD;
    }
    if (health <= 25) {
        return HUD_FACE_CRITICAL;
    }
    if (health <= 50) {
        return HUD_FACE_INJURED;
    }
    if (health <= 75) {
        return HUD_FACE_WOUNDED;
    }
    return HUD_FACE_HEALTHY;
}

void hud_draw_level_clear(SDL_Renderer *renderer, int screen_w, int screen_h, float timer) {
    if (fmodf(timer, 0.5f) < 0.25f) {
        return;
    }
    const char *msg = "LEVEL CLEARED";
    int tw = font_str_px_w(msg);
    int tx = (screen_w - tw) / 2;
    int ty = screen_h / 3 - FONT_CH / 2;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect bar = { tx - 20, ty - 10, tw + 40, FONT_CH + 20 };
    SDL_RenderFillRect(renderer, &bar);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Color gold = { 220, 180, 50, 255 };
    font_draw_string(renderer, msg, tx, ty, gold);
}

void hud_draw_exit_open(SDL_Renderer *renderer, int screen_w, int screen_h) {
    const char *msg = "EXIT OPEN - FIND THE DOOR";
    int tw = font_str_px_w(msg);
    int tx = (screen_w - tw) / 2;
    int ty = 16;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect bar = { tx - 16, ty - 8, tw + 32, FONT_CH + 16 };
    SDL_RenderFillRect(renderer, &bar);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Color gold = { 220, 180, 50, 255 };
    font_draw_string(renderer, msg, tx, ty, gold);
}

void hud_render(SDL_Renderer *renderer, int screen_w, int screen_h, int health, int ammo, int reserve_ammo, int score, int enemies_remaining, const Texture face_tex[HUD_FACE_COUNT]) {
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
    const Texture *face = &face_tex[hud_face_for_health(health)];
    if (face->pixels && face->width > 0 && face->height > 0) {
        for (int y = 0; y < FACE_H; y++) {
            int ty = y * face->height / FACE_H;
            for (int x = 0; x < FACE_W; x++) {
                int tx = x * face->width / FACE_W;
                int idx = (ty * face->width + tx) * 3;
                SDL_SetRenderDrawColor(renderer, face->pixels[idx], face->pixels[idx + 1], face->pixels[idx + 2], 255);
                SDL_RenderDrawPoint(renderer, face_x + x, face_y + y);
            }
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 160, 100, 255);
        SDL_Rect fallback = { face_x, face_y, FACE_W, FACE_H };
        SDL_RenderFillRect(renderer, &fallback);
    }

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

    char reserve_buf[16];
    snprintf(reserve_buf, sizeof(reserve_buf), "| %d", reserve_ammo);
    int reserve_x = ammo_right - font_str_px_w(reserve_buf) - (ammo * (AMMO_BLOCK_W + AMMO_GAP));
    int reserve_y = bar_y + (HUD_HEIGHT - FONT_CH) / 2;
    SDL_Color grey = { 160, 160, 160, 255 };
    font_draw_string(renderer, reserve_buf, reserve_x, reserve_y, grey);

    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "%d", score);
    SDL_Color white = { 220, 220, 220, 255 };
    font_draw_string(renderer, score_buf, bar_x, bar_top + BAR_H + 4, white);

    char enemies_buf[32];
    snprintf(enemies_buf, sizeof(enemies_buf), "ENEMIES %d", enemies_remaining);
    int enemies_x = bar_x + bar_w - font_str_px_w(enemies_buf);
    font_draw_string(renderer, enemies_buf, enemies_x, bar_top + BAR_H + 4, white);
}
