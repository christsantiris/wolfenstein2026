#include "render/sprite.h"
#include <math.h>
#include <stdlib.h>

#define FOV_FACTOR 0.66f

void sprite_render_all(SDL_Renderer *renderer, const Player *p, const EnemyList *el, const float *zbuf, const Texture enemy_tex[][ENEMY_SPRITE_FRAMES], int screen_w, int screen_h) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y =  dir_x * FOV_FACTOR;
    float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);

    for (int i = 0; i < el->count; i++) {
        const Enemy *e = &el->enemies[i];
        if (!e->active) {
            continue;
        }

        float view_angle = atan2f(e->y - p->y, e->x - p->x);
        float rel = view_angle - e->angle;
        while (rel <  0.0f)                { rel += 2.0f * (float)M_PI; }
        while (rel >= 2.0f * (float)M_PI)  { rel -= 2.0f * (float)M_PI; }
        int dir_idx = (int)(rel / ((float)M_PI / 4.0f) + 0.5f) % 8;
        int frame_idx;
        if (e->state == ENEMY_ATTACK) {
            frame_idx = ENEMY_SPRITE_ATTACK;
        } else {
            frame_idx = (e->walk_frame ? ENEMY_SPRITE_WALK_B : 0) + dir_idx;
        }
        const Texture *sprite_tex = &enemy_tex[e->type][frame_idx];

        float ex = e->x - p->x;
        float ey = e->y - p->y;

        float transform_x = inv_det * ( dir_y * ex - dir_x * ey);
        float transform_y = inv_det * (-plane_y * ex + plane_x * ey);

        if (transform_y <= 0.1f) {
            continue;
        }

        int screen_x = (int)((screen_w / 2) * (1.0f + transform_x / transform_y));

        float sprite_scale = (e->type == ENEMY_TYPE_BOSS) ? 1.35f : 1.0f;
        int sprite_h = abs((int)(screen_h * sprite_scale / transform_y));
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

        for (int x = draw_x0; x < draw_x1; x++) {
            if (transform_y >= zbuf[x]) {
                continue;
            }
            float u = (float)(x - (screen_x - sprite_w / 2)) / sprite_w;
            for (int y = draw_y0; y < draw_y1; y++) {
                float v = (float)(y - tex_y_base) / sprite_h;
                unsigned int colour = texture_sample(sprite_tex, u, v);
                unsigned char r = (colour >> 16) & 0xFF;
                unsigned char g = (colour >> 8)  & 0xFF;
                unsigned char b =  colour        & 0xFF;
                if (r == 255 && g == 0 && b == 255) {
                    continue;
                }
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        if (e->type == ENEMY_TYPE_BOSS && screen_x >= 0 && screen_x < screen_w && transform_y < zbuf[screen_x]) {
            const EnemyDef *def = enemy_def(e->type);
            int max_health = def->max_health;
            if (e->health > max_health) {
                max_health = e->health;
            }

            int bar_w = sprite_w * 3 / 5;
            if (bar_w < 36) { bar_w = 36; }
            if (bar_w > 140) { bar_w = 140; }
            int bar_h = 6;
            int bar_x = screen_x - bar_w / 2;
            int bar_y = draw_y0 - 12;
            if (bar_y < 2) { bar_y = 2; }

            int fill_w = (max_health > 0) ? (bar_w - 2) * e->health / max_health : 0;
            if (fill_w < 0) { fill_w = 0; }
            if (fill_w > bar_w - 2) { fill_w = bar_w - 2; }

            SDL_Rect bg = { bar_x, bar_y, bar_w, bar_h };
            SDL_Rect fill = { bar_x + 1, bar_y + 1, fill_w, bar_h - 2 };
            SDL_SetRenderDrawColor(renderer, 20, 10, 12, 255);
            SDL_RenderFillRect(renderer, &bg);
            SDL_SetRenderDrawColor(renderer, 190, 24, 28, 255);
            SDL_RenderFillRect(renderer, &fill);
            SDL_SetRenderDrawColor(renderer, 230, 210, 160, 255);
            SDL_RenderDrawRect(renderer, &bg);
        }
    }
}
