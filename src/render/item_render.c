#include "render/item_render.h"
#include <math.h>

#define FOV_FACTOR 0.66f

void item_render_all(SDL_Renderer *renderer, const Player *p, const ItemList *il, const float *zbuf, const Texture *ammo_tex, const Texture *health_tex, const Texture *weapon_kit_tex, const Texture *weapon_kit_ak47_tex, const Texture *weapon_kit_dual_tex, int screen_w, int screen_h) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y =  dir_x * FOV_FACTOR;
    float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);

    for (int i = 0; i < il->count; i++) {
        const Item *it = &il->items[i];
        if (!it->active) { continue; }

        float ex = it->x - p->x;
        float ey = it->y - p->y;

        float transform_x = inv_det * ( dir_y * ex - dir_x * ey);
        float transform_y = inv_det * (-plane_y * ex + plane_x * ey);

        if (transform_y <= 0.1f) { continue; }

        int screen_x = (int)((screen_w / 2) * (1.0f + transform_x / transform_y));

        int sprite_h = abs((int)(screen_h / transform_y));
        int draw_y0 = (screen_h - sprite_h) / 2;
        int draw_y1 = (screen_h + sprite_h) / 2;
        if (draw_y0 < 0) { draw_y0 = 0; }
        if (draw_y1 > screen_h) { draw_y1 = screen_h; }

        int sprite_w = sprite_h;
        int draw_x0 = screen_x - sprite_w / 2;
        int draw_x1 = screen_x + sprite_w / 2;
        if (draw_x0 < 0) { draw_x0 = 0; }
        if (draw_x1 > screen_w) { draw_x1 = screen_w; }

        int tex_y_base = screen_h / 2 - sprite_h / 2;
        const Texture *tex = (it->type == ITEM_AMMO) ? ammo_tex
                           : (it->type == ITEM_HEALTH) ? health_tex
                           : (it->type == ITEM_WEAPON_KIT_AK47) ? weapon_kit_ak47_tex
                           : (it->type == ITEM_WEAPON_KIT_DUAL) ? weapon_kit_dual_tex
                           : weapon_kit_tex;

        for (int x = draw_x0; x < draw_x1; x++) {
            if (transform_y >= zbuf[x]) { continue; }
            float u = (float)(x - (screen_x - sprite_w / 2)) / sprite_w;
            for (int y = draw_y0; y < draw_y1; y++) {
                float v = (float)(y - tex_y_base) / sprite_h;
                unsigned int colour = texture_sample(tex, u, v);
                unsigned char r = (colour >> 16) & 0xFF;
                unsigned char g = (colour >> 8)  & 0xFF;
                unsigned char b =  colour        & 0xFF;
                if (r == 255 && g == 0 && b == 255) { continue; }
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}
