#include "render/sprite.h"
#include <math.h>
#include <stdlib.h>

#define FOV_FACTOR 0.66f
#define CORPSE_MIN_RENDER_DISTANCE2 1.0f

static int sprite_project_point(const Player *p, float x, float y, int screen_w, float *depth, int *screen_x) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y = dir_x * FOV_FACTOR;
    float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);
    float rel_x = x - p->x;
    float rel_y = y - p->y;
    float transform_x = inv_det * (dir_y * rel_x - dir_x * rel_y);
    *depth = inv_det * (-plane_y * rel_x + plane_x * rel_y);
    if (*depth <= 0.1f) {
        return 0;
    }
    *screen_x = (int)((screen_w / 2) * (1.0f + transform_x / *depth));
    return 1;
}

static void sprite_draw_grenade_body(SDL_Renderer *renderer, const GrenadeState *grenade, const float *zbuf, float depth, int center_x, int center_y, int screen_w, int screen_h) {
    int radius = (int)(screen_h * 0.075f / depth);
    if (radius < 2) {
        radius = 2;
    }
    if (radius > 18) {
        radius = 18;
    }
    int spin = (int)(grenade->distance * 24.0f) % (radius * 2 + 1);
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int screen_x = center_x + dx;
            int screen_y = center_y + dy;
            if (screen_x < 0 || screen_x >= screen_w || screen_y < 0 || screen_y >= screen_h || depth >= zbuf[screen_x]) {
                continue;
            }
            float nx = (float)dx / radius;
            float ny = (float)dy / radius;
            float dist2 = nx * nx + ny * ny;
            if (dist2 > 1.0f) {
                continue;
            }
            if (dist2 > 0.72f) {
                SDL_SetRenderDrawColor(renderer, 24, 25, 18, 255);
            } else if ((dy + spin) % (radius + 1) == 0 || (dy + spin + 1) % (radius + 1) == 0) {
                SDL_SetRenderDrawColor(renderer, 185, 132, 48, 255);
            } else if (dx < 0) {
                SDL_SetRenderDrawColor(renderer, 92, 99, 53, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 55, 61, 34, 255);
            }
            SDL_RenderDrawPoint(renderer, screen_x, screen_y);
        }
    }
}

static void sprite_draw_explosion(SDL_Renderer *renderer, const GrenadeState *grenade, const float *zbuf, float depth, int center_x, int center_y, int screen_w, int screen_h) {
    float progress = 1.0f - grenade->explosion_timer / GRENADE_EXPLOSION_DURATION;
    float pulse = sinf(progress * (float)M_PI);
    int radius = (int)(screen_h * (0.08f + pulse * 0.34f) / depth);
    if (radius < 5) {
        radius = 5;
    }
    if (radius > 150) {
        radius = 150;
    }
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int screen_x = center_x + dx;
            int screen_y = center_y + dy;
            if (screen_x < 0 || screen_x >= screen_w || screen_y < 0 || screen_y >= screen_h || depth >= zbuf[screen_x]) {
                continue;
            }
            float nx = (float)dx / radius;
            float ny = (float)dy / radius;
            float radial = sqrtf(nx * nx + ny * ny);
            if (radial > 1.0f) {
                continue;
            }
            if (progress < 0.22f && radial < 0.72f) {
                SDL_SetRenderDrawColor(renderer, 255, 245, 190, 255);
            } else if (progress < 0.68f && radial < 0.62f) {
                SDL_SetRenderDrawColor(renderer, 255, 164, 35, 255);
            } else if (progress < 0.68f) {
                SDL_SetRenderDrawColor(renderer, 190, 54, 16, 255);
            } else if (((dx * 17 + dy * 31) & 7) < 5) {
                SDL_SetRenderDrawColor(renderer, 74, 69, 63, 255);
            } else {
                continue;
            }
            SDL_RenderDrawPoint(renderer, screen_x, screen_y);
        }
    }
}

void sprite_render_grenade(SDL_Renderer *renderer, const Player *p, const GrenadeState *grenade, const float *zbuf, int screen_w, int screen_h) {
    float depth;
    int screen_x;
    if (grenade->active && sprite_project_point(p, grenade->x, grenade->y, screen_w, &depth, &screen_x)) {
        sprite_draw_grenade_body(renderer, grenade, zbuf, depth, screen_x, screen_h / 2, screen_w, screen_h);
    }
    if (grenade->explosion_timer > 0.0f && sprite_project_point(p, grenade->explosion_x, grenade->explosion_y, screen_w, &depth, &screen_x)) {
        sprite_draw_explosion(renderer, grenade, zbuf, depth, screen_x, screen_h / 2, screen_w, screen_h);
    }
}

void sprite_render_all(SDL_Renderer *renderer, const Player *p, const EnemyList *el, const float *zbuf, const Texture enemy_tex[][ENEMY_SPRITE_FRAMES], int difficulty, int screen_w, int screen_h) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y =  dir_x * FOV_FACTOR;
    float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);

    for (int i = 0; i < el->count; i++) {
        const Enemy *e = &el->enemies[i];

        float view_angle = atan2f(e->y - p->y, e->x - p->x);
        float rel = view_angle - e->angle;
        while (rel <  0.0f)                { rel += 2.0f * (float)M_PI; }
        while (rel >= 2.0f * (float)M_PI)  { rel -= 2.0f * (float)M_PI; }
        int dir_idx = (int)(rel / ((float)M_PI / 4.0f) + 0.5f) % 8;
        int frame_idx;
        if (!e->active) {
            frame_idx = ENEMY_SPRITE_CORPSE;
        } else if (e->state == ENEMY_ATTACK) {
            frame_idx = e->attack_flash_timer > 0.0f ? ENEMY_SPRITE_FIRE : ENEMY_SPRITE_AIM;
        } else {
            frame_idx = (e->walk_frame ? ENEMY_SPRITE_WALK_B : 0) + dir_idx;
        }
        const Texture *sprite_tex = &enemy_tex[e->type][frame_idx];

        float ex = e->x - p->x;
        float ey = e->y - p->y;
        if (!e->active && ex * ex + ey * ey < CORPSE_MIN_RENDER_DISTANCE2) {
            continue;
        }

        float transform_x = inv_det * ( dir_y * ex - dir_x * ey);
        float transform_y = inv_det * (-plane_y * ex + plane_x * ey);

        if (transform_y <= 0.1f) {
            continue;
        }

        int screen_x = (int)((screen_w / 2) * (1.0f + transform_x / transform_y));

        float sprite_scale = 1.0f;
        if (e->type == ENEMY_TYPE_BOSS) {
            sprite_scale = 1.35f;
        } else if (e->type == ENEMY_TYPE_MINIBOSS) {
            sprite_scale = 1.18f;
        } else if (e->type == ENEMY_TYPE_DOG) {
            sprite_scale = 0.72f;
        }
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

        int is_boss = e->type == ENEMY_TYPE_BOSS || e->type == ENEMY_TYPE_MINIBOSS;
        if (e->active && is_boss && screen_x >= 0 && screen_x < screen_w && transform_y < zbuf[screen_x]) {
            int max_health = enemy_max_health(e->type, difficulty);

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
