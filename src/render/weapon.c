#include "render/weapon.h"
#include <math.h>

#define WEAPON_SCALE  3
#define WHIP_DURATION 0.4f
#define WHIP_SWING_X  90
#define WHIP_SWING_Y  40

void weapon_render(SDL_Renderer *renderer, const Texture *tex, float shot_timer, float whip_timer, int screen_w, int screen_h) {
    int w = tex->width  * WEAPON_SCALE;
    int h = tex->height * WEAPON_SCALE;
    int x0 = (screen_w - w) / 2;
    int y0 = screen_h - h;

    if (whip_timer > 0.0f) {
        float t = 1.0f - (whip_timer / WHIP_DURATION);
        float swing = sinf(t * 3.14159f);
        x0 += (int)(swing * WHIP_SWING_X);
        y0 += (int)(swing * WHIP_SWING_Y);
    }

    for (int sy = 0; sy < h; sy++) {
        int ty = sy / WEAPON_SCALE;
        for (int sx = 0; sx < w; sx++) {
            int tx = sx / WEAPON_SCALE;
            int idx = (ty * tex->width + tx) * 3;
            unsigned char r = tex->pixels[idx];
            unsigned char g = tex->pixels[idx + 1];
            unsigned char b = tex->pixels[idx + 2];
            if (r == 255 && g == 0 && b == 255) { continue; }
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x0 + sx, y0 + sy);
        }
    }

    if (shot_timer > 0.0f) {
        int fx = x0 + w / 2 - 12;
        int fy = y0 - 30;
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        SDL_Rect flash = { fx, fy, 24, 30 };
        SDL_RenderFillRect(renderer, &flash);
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
        SDL_Rect core = { fx + 7, fy + 4, 10, 18 };
        SDL_RenderFillRect(renderer, &core);
    }
}
