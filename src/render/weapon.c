#include "render/weapon.h"
#include <math.h>
#include <stdlib.h>

#define WEAPON_SCALE 3.0f
#define WHIP_DURATION 0.4f
#define WHIP_SWING_X 90
#define WHIP_SWING_Y 40

typedef struct {
    int muzzle_x;
    int muzzle_y;
    int alternate_muzzle_x;
    int recoil_x;
    int recoil_y;
    float scale_kick;
    int flash_radius;
    unsigned char flash_r;
    unsigned char flash_g;
    unsigned char flash_b;
} WeaponAnimDef;

static const WeaponAnimDef WEAPON_ANIMS[GUN_COUNT] = {
    [GUN_9MM_HANDGUN] = { 32, 6, 32, 1, 9, 0.12f, 14, 255, 150, 35 },
    [GUN_DUAL_HANDGUN] = { 25, 11, 38, 0, 8, 0.06f, 12, 255, 165, 45 },
    [GUN_SHOTGUN] = { 31, -6, 31, 1, 18, 0.22f, 22, 255, 120, 25 },
    [GUN_AK47] = { 24, 2, 24, 5, 16, 0.16f, 22, 255, 135, 25 },
    [GUN_BATTLE_RIFLE] = { 24, 3, 24, 2, 12, 0.15f, 17, 255, 185, 60 },
    [GUN_KNIFE] = { 32, 0, 32, 0, 0, 0.0f, 0, 0, 0, 0 },
    [GUN_RIFLE_GRENADE] = { 27, 2, 27, 0, 24, 0.28f, 28, 255, 110, 20 },
    [GUN_UZI] = { 29, 5, 29, 3, 10, 0.10f, 14, 255, 155, 35 },
};

static float weapon_recoil(const WeaponDef *weapon, float shot_cooldown) {
    if (weapon->type == GUN_KNIFE || weapon->shot_cooldown <= 0.0f || shot_cooldown <= 0.0f) {
        return 0.0f;
    }
    float remaining = shot_cooldown / weapon->shot_cooldown;
    if (remaining > 1.0f) {
        remaining = 1.0f;
    }
    float elapsed = 1.0f - remaining;
    if (elapsed < 0.18f) {
        return elapsed / 0.18f;
    }
    float recovery = (elapsed - 0.18f) / 0.82f;
    if (recovery > 1.0f) {
        recovery = 1.0f;
    }
    return 1.0f - recovery * recovery;
}

static void draw_flash_diamond(SDL_Renderer *renderer, int cx, int cy, int radius, unsigned char r, unsigned char g, unsigned char b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    for (int y = -radius; y <= radius; y++) {
        int half_width = radius - abs(y);
        SDL_RenderDrawLine(renderer, cx - half_width, cy + y, cx + half_width, cy + y);
    }
}

static void draw_muzzle_flash(SDL_Renderer *renderer, int cx, int cy, const WeaponAnimDef *anim) {
    draw_flash_diamond(renderer, cx, cy, anim->flash_radius, anim->flash_r, anim->flash_g, anim->flash_b);
    int core_radius = anim->flash_radius / 2;
    draw_flash_diamond(renderer, cx, cy, core_radius, 255, 245, 170);
    SDL_SetRenderDrawColor(renderer, 255, 255, 225, 255);
    SDL_RenderDrawLine(renderer, cx, cy - anim->flash_radius - 5, cx, cy + anim->flash_radius + 5);
    SDL_RenderDrawLine(renderer, cx - anim->flash_radius - 5, cy, cx + anim->flash_radius + 5, cy);
}

void weapon_render(SDL_Renderer *renderer, const WeaponDef *weapon, const Texture *tex, const Texture reload_tex[WEAPON_RELOAD_FRAME_COUNT], int ammo, float shot_timer, float shot_cooldown, float whip_timer, int is_reloading, float reload_timer, int screen_w, int screen_h) {
    const WeaponAnimDef *anim = &WEAPON_ANIMS[weapon->type];
    if (is_reloading && weapon->type != GUN_KNIFE && weapon->reload_time > 0.0f) {
        float progress = 1.0f - reload_timer / weapon->reload_time;
        if (progress < 0.0f) {
            progress = 0.0f;
        }
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        int frame = (int)(progress * WEAPON_RELOAD_FRAME_COUNT);
        if (frame >= WEAPON_RELOAD_FRAME_COUNT) {
            frame = WEAPON_RELOAD_FRAME_COUNT - 1;
        }
        tex = &reload_tex[frame];
    }
    float recoil = whip_timer > 0.0f || is_reloading ? 0.0f : weapon_recoil(weapon, shot_cooldown);
    float scale = WEAPON_SCALE + recoil * anim->scale_kick;
    int w = (int)(tex->width * scale);
    int h = (int)(tex->height * scale);
    int x0 = (screen_w - w) / 2;
    int y0 = screen_h - h;

    if (weapon->type != GUN_DUAL_HANDGUN) {
        y0 -= (int)(recoil * anim->recoil_y);
    }
    int recoil_direction = (ammo & 1) ? 1 : -1;
    x0 += (int)(recoil * anim->recoil_x * recoil_direction);

    if (whip_timer > 0.0f) {
        float t = 1.0f - (whip_timer / WHIP_DURATION);
        float swing = sinf(t * 3.14159f);
        x0 += (int)(swing * WHIP_SWING_X);
        y0 += (int)(swing * WHIP_SWING_Y);
    }

    int dual_right_fired = (ammo & 1) != 0;
    if (shot_timer > 0.0f && whip_timer <= 0.0f && anim->flash_radius > 0) {
        int muzzle_x = weapon->type == GUN_DUAL_HANDGUN && dual_right_fired ? anim->alternate_muzzle_x : anim->muzzle_x;
        int flash_x = x0 + (int)(muzzle_x * scale);
        int flash_y = y0 + (int)(anim->muzzle_y * scale);
        if (weapon->type == GUN_DUAL_HANDGUN) {
            flash_y -= (int)(recoil * anim->recoil_y);
        }
        draw_muzzle_flash(renderer, flash_x, flash_y, anim);
    }

    for (int sy = 0; sy < h; sy++) {
        int ty = sy * tex->height / h;
        for (int sx = 0; sx < w; sx++) {
            int tx = sx * tex->width / w;
            int idx = (ty * tex->width + tx) * 3;
            unsigned char r = tex->pixels[idx];
            unsigned char g = tex->pixels[idx + 1];
            unsigned char b = tex->pixels[idx + 2];
            if (r == 255 && g == 0 && b == 255) { continue; }
            int draw_y = y0 + sy;
            if (weapon->type == GUN_DUAL_HANDGUN) {
                int source_is_right = tx >= tex->width / 2;
                if (source_is_right == dual_right_fired) {
                    draw_y -= (int)(recoil * anim->recoil_y);
                }
            }
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x0 + sx, draw_y);
        }
    }
}
