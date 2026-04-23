#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "player.h"
#include "raycaster.h"
#include "input.h"
#include "texture.h"

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

    Texture wall_tex;
    if (texture_create(&wall_tex, 64, 64) != 0) {
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_brick(&wall_tex);

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
        }

        input_update(&player, &map, dt);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        raycaster_render(renderer, &map, &player, &wall_tex, w, h);
        SDL_RenderPresent(renderer);
    }

    texture_free(&wall_tex);
    map_free(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
