#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/map.h"
#include "core/player.h"
#include "core/game.h"
#include "render/raycaster.h"
#include "render/texture.h"
#include "render/minimap.h"
#include "render/hud.h"
#include "render/sprite.h"
#include "render/weapon.h"
#include "input/input.h"

#define SCREEN_W 800
#define SCREEN_H 600

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Wolfenstein 2026",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Map map;
    if (map_load(&map, "assets/maps/level1.map") != 0) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player;
    player_init(&player, 14.5f, 10.5f, 0.0f);

    GameState game;
    game_init(&game);

    Texture wall_tex;
    if (texture_create(&wall_tex, 64, 64) != 0) {
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_brick(&wall_tex);

    Texture pistol_tex;
    if (texture_load_ppm(&pistol_tex, "assets/sprites/pistol.ppm") != 0) {
        texture_create(&pistol_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { pistol_tex.pixels[i] = 0; }
    }

    Texture guard_tex;
    if (texture_load_ppm(&guard_tex, "assets/sprites/guard_front.ppm") != 0) {
        texture_create(&guard_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i += 3) {
            guard_tex.pixels[i]     = 150;
            guard_tex.pixels[i + 1] = 110;
            guard_tex.pixels[i + 2] = 60;
        }
    }

    int zbuf_w = 0;
    float *zbuf = NULL;

    int running = 1;
    SDL_Event e;
    Uint32 last_ticks = SDL_GetTicks();
    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                game_shoot(&game, &player);
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                game_shoot(&game, &player);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                game_reload(&game);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_o) {
                int door_x = (int)(player.x + cosf(player.angle));
                int door_y = (int)(player.y + sinf(player.angle));
                if (map_is_door(&map, door_x, door_y)) {
                    map_toggle_door(&map, door_x, door_y);
                }
            }
        }

        input_update(&player, &map, dt);
        game_update_enemies(&game, &player, &map, dt);
        game_update_timers(&game, dt);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        if (w != zbuf_w) {
            zbuf = realloc(zbuf, w * sizeof(float));
            zbuf_w = w;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        raycaster_render(renderer, &map, &player, &wall_tex, zbuf, w, h - HUD_HEIGHT);
        sprite_render_all(renderer, &player, &game.enemies, zbuf, &guard_tex, w, h - HUD_HEIGHT);
        weapon_render(renderer, &pistol_tex, game.shot_timer, w, h - HUD_HEIGHT);
        minimap_render(renderer, &map, &player);
        hud_render(renderer, w, h, game.health, game.ammo);
        if (game.hit_flash_timer > 0.0f) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 180, 0, 0, 100);
            SDL_Rect flash = { 0, 0, w, h };
            SDL_RenderFillRect(renderer, &flash);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
        SDL_RenderPresent(renderer);
    }

    free(zbuf);
    texture_free(&pistol_tex);
    texture_free(&wall_tex);
    map_free(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
