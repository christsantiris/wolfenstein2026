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
#include "ui/menu.h"
#include "ui/landing.h"

#define SCREEN_W 800
#define SCREEN_H 600

typedef enum {
    APP_LANDING,
    APP_DIFFICULTY,
    APP_PLAYING
} AppState;

static int start_game(Map *map, Player *player, GameState *game, int level) {
    char path[64];
    snprintf(path, sizeof(path), "assets/maps/level%d.map", level);
    map_free(map);
    if (map_load(map, path) != 0) {
        return -1;
    }
    player_init(player, 14.5f, 10.5f, 0.0f);
    game_init(game);
    enemy_list_init(&game->enemies, map, level, player->x, player->y);
    return 0;
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    srand((unsigned int)SDL_GetTicks());

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

    Texture door_tex;
    if (texture_create(&door_tex, 64, 64) != 0) {
        texture_free(&wall_tex);
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_door(&door_tex);

    Texture exit_tex;
    if (texture_create(&exit_tex, 64, 64) != 0) {
        texture_free(&door_tex);
        texture_free(&wall_tex);
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_exit_door(&exit_tex);

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

    AppState app_state = APP_LANDING;
    Menu menu = { 0 };
    int current_level = 1;
    int game_over = 0;
    int running = 1;
    SDL_Event e;
    Uint32 last_ticks = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }

            if (app_state == APP_LANDING) {
                LandingResult lr = landing_handle_event(&e, w, h);
                if (lr == LANDING_NEW_GAME) {
                    difficulty_screen_reset();
                    app_state = APP_DIFFICULTY;
                } else if (lr == LANDING_QUIT) {
                    running = 0;
                }
            } else if (app_state == APP_DIFFICULTY) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    app_state = APP_LANDING;
                }
                Difficulty d = difficulty_screen_handle_event(&e, w, h);
                if (d != DIFF_COUNT) {
                    current_level = 1;
                    start_game(&map, &player, &game, current_level);
                    game_over = 0;
                    menu.is_open = 0;
                    app_state = APP_PLAYING;
                }
            } else if (app_state == APP_PLAYING) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    menu.is_open = !menu.is_open;
                }
                if (menu.is_open) {
                    menu_handle_event(&menu, &e, &running);
                } else if (!game_over) {
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
                } else {
                    GameOverResult result = game_over_handle_event(&e, w, h);
                    if (result == GAME_OVER_QUIT) {
                        running = 0;
                    } else if (result == GAME_OVER_NEW_GAME) {
                        landing_reset();
                        app_state = APP_LANDING;
                        game_over = 0;
                    }
                }
            }
        }

        if (app_state == APP_PLAYING && !menu.is_open && !game_over) {
            input_update(&player, &map, dt);
            game_update_enemies(&game, &player, &map, dt);
            game_update_timers(&game, dt);
            if (game.health <= 0) {
                game_over = 1;
            }
            if (enemy_list_all_dead(&game.enemies)) {
                map_unlock_exits(&map);
            }
            if (map_cell(&map, (int)player.x, (int)player.y) == MAP_CELL_EXIT_OPEN) {
                current_level++;
                if (start_game(&map, &player, &game, current_level) != 0) {
                    current_level = 1;
                    landing_reset();
                    app_state = APP_LANDING;
                }
            }
        }

        if (w != zbuf_w) {
            zbuf = realloc(zbuf, w * sizeof(float));
            zbuf_w = w;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (app_state == APP_LANDING) {
            landing_render(renderer, w, h);
        } else if (app_state == APP_DIFFICULTY) {
            difficulty_screen_render(renderer, w, h);
        } else {
            raycaster_render(renderer, &map, &player, &wall_tex, &door_tex, &exit_tex, zbuf, w, h - HUD_HEIGHT);
            sprite_render_all(renderer, &player, &game.enemies, zbuf, &guard_tex, w, h - HUD_HEIGHT);
            weapon_render(renderer, &pistol_tex, game.shot_timer, w, h - HUD_HEIGHT);
            minimap_render(renderer, &map, &player);
            hud_render(renderer, w, h, game.health, game.ammo);
            if (game.level_clear_timer > 0.0f) {
                hud_draw_level_clear(renderer, w, h - HUD_HEIGHT, game.level_clear_timer);
            }
            if (game.hit_flash_timer > 0.0f) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 180, 0, 0, 100);
                SDL_Rect flash = { 0, 0, w, h };
                SDL_RenderFillRect(renderer, &flash);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            if (menu.is_open) {
                menu_render(renderer, &menu, w, h);
            }
            if (game_over) {
                game_over_render(renderer, w, h);
            }
        }

        SDL_RenderPresent(renderer);
    }

    free(zbuf);
    texture_free(&pistol_tex);
    texture_free(&exit_tex);
    texture_free(&door_tex);
    texture_free(&wall_tex);
    map_free(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
