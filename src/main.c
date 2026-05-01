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
#include "render/item_render.h"
#include "render/weapon.h"
#include "input/input.h"
#include "ui/menu.h"
#include "ui/landing.h"
#include "ui/highscore.h"
#include "audio/sound.h"

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
    int saved_score = game->score;
    int saved_has_weapon[GUN_COUNT];
    int saved_ammo_per_gun[GUN_COUNT];
    WeaponDef saved_weapon = game->current_weapon;
    for (int i = 0; i < GUN_COUNT; i++) {
        saved_has_weapon[i] = game->has_weapon[i];
        saved_ammo_per_gun[i] = game->ammo_per_gun[i];
    }
    game_init(game);
    game->score = saved_score;
    for (int i = 0; i < GUN_COUNT; i++) {
        game->has_weapon[i] = saved_has_weapon[i];
        game->ammo_per_gun[i] = saved_ammo_per_gun[i];
    }
    game->current_weapon = saved_weapon;
    game->ammo = game->ammo_per_gun[saved_weapon.type];
    enemy_list_init(&game->enemies, map, level, player->x, player->y);
    item_list_init(&game->items, map, level, player->x, player->y);
    return 0;
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    Mix_Init(MIX_INIT_MP3);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
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

    Texture wall_tex[3];
    if (texture_create(&wall_tex[0], 64, 64) != 0 ||
        texture_create(&wall_tex[1], 64, 64) != 0 ||
        texture_create(&wall_tex[2], 64, 64) != 0) {
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_brick(&wall_tex[0]);
    texture_generate_stone(&wall_tex[1]);
    texture_generate_sandstone(&wall_tex[2]);

    Texture door_tex;
    if (texture_create(&door_tex, 64, 64) != 0) {
        for (int wl = 2; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
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
        for (int wl = 2; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_exit_door(&exit_tex);

    Texture ammo_pickup_tex;
    texture_create(&ammo_pickup_tex, 64, 64);
    texture_generate_ammo_pickup(&ammo_pickup_tex);

    Texture health_pickup_tex;
    texture_create(&health_pickup_tex, 64, 64);
    texture_generate_health_pickup(&health_pickup_tex);

    Texture weapon_kit_tex;
    texture_create(&weapon_kit_tex, 64, 64);
    texture_generate_weapon_kit(&weapon_kit_tex);

    Texture shotgun_tex;
    texture_create(&shotgun_tex, 64, 64);
    texture_generate_shotgun(&shotgun_tex);

    Texture pistol_tex;
    if (texture_load_ppm(&pistol_tex, "assets/sprites/pistol.ppm") != 0) {
        texture_create(&pistol_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { pistol_tex.pixels[i] = 0; }
    }

    Texture enemy_tex[ENEMY_TYPE_COUNT][8];
    /* Guard */
    if (texture_load_ppm(&enemy_tex[ENEMY_TYPE_GUARD][4], "assets/sprites/guard_front.ppm") != 0) {
        texture_create(&enemy_tex[ENEMY_TYPE_GUARD][4], 64, 64);
        texture_generate_guard_dir(&enemy_tex[ENEMY_TYPE_GUARD][4], 4);
    }
    for (int d = 0; d < 8; d++) {
        if (d == 4) { continue; }
        texture_create(&enemy_tex[ENEMY_TYPE_GUARD][d], enemy_tex[ENEMY_TYPE_GUARD][4].width, enemy_tex[ENEMY_TYPE_GUARD][4].height);
    }
    texture_derive_guard_dirs(enemy_tex[ENEMY_TYPE_GUARD]);
    /* Officer — recolor guard uniform to navy blue */
    texture_recolor_uniform(&enemy_tex[ENEMY_TYPE_OFFICER][4], &enemy_tex[ENEMY_TYPE_GUARD][4], 60, 80, 160);
    for (int d = 0; d < 8; d++) {
        if (d == 4) { continue; }
        texture_create(&enemy_tex[ENEMY_TYPE_OFFICER][d], enemy_tex[ENEMY_TYPE_OFFICER][4].width, enemy_tex[ENEMY_TYPE_OFFICER][4].height);
    }
    texture_derive_guard_dirs(enemy_tex[ENEMY_TYPE_OFFICER]);
    /* SS — recolor guard uniform to black */
    texture_recolor_uniform(&enemy_tex[ENEMY_TYPE_SS][4], &enemy_tex[ENEMY_TYPE_GUARD][4], 35, 35, 40);
    for (int d = 0; d < 8; d++) {
        if (d == 4) { continue; }
        texture_create(&enemy_tex[ENEMY_TYPE_SS][d], enemy_tex[ENEMY_TYPE_SS][4].width, enemy_tex[ENEMY_TYPE_SS][4].height);
    }
    texture_derive_guard_dirs(enemy_tex[ENEMY_TYPE_SS]);

    int zbuf_w = 0;
    float *zbuf = NULL;

    Sound gun_sounds[GUN_COUNT] = { 0 };
    Sound reload_sounds[GUN_COUNT] = { 0 };
    for (int gi = 0; gi < GUN_COUNT; gi++) {
        const WeaponDef *wd = weapon_def((GunType)gi);
        sound_load(&gun_sounds[gi], wd->sound_path);
        if (wd->reload_sound_path) {
            sound_load(&reload_sounds[gi], wd->reload_sound_path);
        }
    }
    Sound whip_sound = { 0 };
    sound_load(&whip_sound, "assets/sounds/punch.mp3");
    Sound door_sound = { 0 };
    sound_load(&door_sound, "assets/sounds/dooropen.mp3");
    Sound level_sound = { 0 };
    sound_load(&level_sound, "assets/sounds/nextlevel.mp3");
    Sound enemy_sound = { 0 };
    sound_load(&enemy_sound, "assets/sounds/die.mp3");

    Music level_music[3] = { 0 };
    music_load(&level_music[0], "assets/music/level1theme.mp3");
    music_load(&level_music[1], "assets/music/level2theme.mp3");
    music_load(&level_music[2], "assets/music/level3theme.mp3");

    HighScoreTable hs_table;
    highscore_load(&hs_table);
    int hs_rank = 0;

    AppState app_state = APP_LANDING;
    Menu menu = { 0 };
    menu.music_on = 1;
    menu.sound_on = 1;
    int current_level = 1;
    int game_over = 0;
    int show_minimap = 1;
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
                    game.score = 0;
                    start_game(&map, &player, &game, current_level);
                    music_play(&level_music[current_level - 1]);
                    game_over = 0;
                    menu.is_open = 0;
                    app_state = APP_PLAYING;
                }
            } else if (app_state == APP_PLAYING) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    menu.is_open = !menu.is_open;
                }
                if (menu.is_open) {
                    MenuAction action = menu_handle_event(&menu, &e, w, h);
                    if (action == MENU_ACTION_QUIT) {
                        running = 0;
                    } else if (action == MENU_ACTION_NEW_GAME) {
                        menu.is_open = 0;
                        music_stop();
                        landing_reset();
                        app_state = APP_LANDING;
                    } else if (action == MENU_ACTION_MUSIC_TOGGLE) {
                        if (menu.music_on) { Mix_ResumeMusic(); } else { Mix_PauseMusic(); }
                    } else if (action == MENU_ACTION_SOUND_TOGGLE) {
                        sound_set_enabled(menu.sound_on);
                    }
                } else if (!game_over) {
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                        if (game_shoot(&game, &player)) {
                            sound_play(&gun_sounds[game.current_weapon.type]);
                        } else if (game.ammo == 0) {
                            if (game_pistol_whip(&game, &player)) {
                                sound_play(&whip_sound);
                            }
                        }
                    }
                    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                        if (game_shoot(&game, &player)) {
                            sound_play(&gun_sounds[game.current_weapon.type]);
                        } else if (game.ammo == 0) {
                            if (game_pistol_whip(&game, &player)) {
                                sound_play(&whip_sound);
                            }
                        }
                    }
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                        if (game_reload(&game)) {
                            sound_play(&reload_sounds[game.current_weapon.type]);
                        }
                    }
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
                        show_minimap = !show_minimap;
                    }
                    if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_c || e.key.keysym.sym == SDLK_w)) {
                        game_cycle_weapon(&game);
                    }
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_o) {
                        int door_x = (int)(player.x + cosf(player.angle));
                        int door_y = (int)(player.y + sinf(player.angle));
                        if (map_is_door(&map, door_x, door_y)) {
                            map_toggle_door(&map, door_x, door_y);
                            sound_play(&door_sound);
                        }
                    }
                } else {
                    GameOverResult result = game_over_handle_event(&e, w, h, hs_table.count);
                    if (result == GAME_OVER_QUIT) {
                        running = 0;
                    } else if (result == GAME_OVER_NEW_GAME) {
                        music_stop();
                        landing_reset();
                        app_state = APP_LANDING;
                        game_over = 0;
                    }
                }
            }
        }

        if (app_state == APP_PLAYING && !menu.is_open && !game_over) {
            input_update(&player, &map, dt);
            if (game_update_enemies(&game, &player, &map, dt)) {
                sound_play(&enemy_sound);
            }
            game_update_timers(&game, dt);
            if (game.health <= 0) {
                game_over = 1;
                hs_rank = highscore_insert(&hs_table, game.score);
                highscore_save(&hs_table);
            }
            for (int ii = 0; ii < game.items.count; ii++) {
                Item *it = &game.items.items[ii];
                if (!it->active) { continue; }
                float dx = player.x - it->x;
                float dy = player.y - it->y;
                if (dx * dx + dy * dy < 0.5f * 0.5f) {
                    if (it->type == ITEM_AMMO) {
                        game.reserve_ammo += AMMO_PICKUP_AMOUNT;
                        if (game.reserve_ammo > AMMO_RESERVE_MAX) {
                            game.reserve_ammo = AMMO_RESERVE_MAX;
                        }
                    } else if (it->type == ITEM_HEALTH) {
                        game.health += HEALTH_PICKUP_AMOUNT;
                        if (game.health > 100) { game.health = 100; }
                    } else if (it->type == ITEM_WEAPON_KIT) {
                        if (!game.has_weapon[GUN_SHOTGUN]) {
                            game.has_weapon[GUN_SHOTGUN] = 1;
                            game.ammo_per_gun[GUN_SHOTGUN] = weapon_def(GUN_SHOTGUN)->max_ammo;
                            game_cycle_weapon(&game);
                        }
                    }
                    it->active = 0;
                }
            }
            if (enemy_list_all_dead(&game.enemies)) {
                map_unlock_exits(&map);
            }
            if (map_cell(&map, (int)player.x, (int)player.y) == MAP_CELL_EXIT_OPEN) {
                current_level++;
                if (start_game(&map, &player, &game, current_level) != 0) {
                    current_level = 1;
                    music_stop();
                    landing_reset();
                    app_state = APP_LANDING;
                } else {
                    sound_play(&level_sound);
                    int idx = current_level - 1;
                    if (idx >= 0 && idx < 3) {
                        music_play(&level_music[idx]);
                    }
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
            int wall_idx = (current_level - 1 < 3) ? current_level - 1 : 2;
            raycaster_render(renderer, &map, &player, &wall_tex[wall_idx], &door_tex, &exit_tex, zbuf, w, h - HUD_HEIGHT);
            sprite_render_all(renderer, &player, &game.enemies, zbuf, enemy_tex, w, h - HUD_HEIGHT);
            item_render_all(renderer, &player, &game.items, zbuf, &ammo_pickup_tex, &health_pickup_tex, &weapon_kit_tex, w, h - HUD_HEIGHT);
            const Texture *weapon_textures[GUN_COUNT] = { &pistol_tex, &shotgun_tex };
            weapon_render(renderer, weapon_textures[game.current_weapon.type], game.shot_timer, game.pistol_whip_timer, w, h - HUD_HEIGHT);
            if (show_minimap) { minimap_render(renderer, &map, &player); }
            hud_render(renderer, w, h, game.health, game.ammo, game.reserve_ammo, game.score);
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
                game_over_render(renderer, w, h, game.score, hs_rank, &hs_table);
            }
        }

        SDL_RenderPresent(renderer);
    }

    free(zbuf);
    music_stop();
    for (int m = 0; m < 3; m++) { music_free(&level_music[m]); }
    sound_free(&enemy_sound);
    sound_free(&level_sound);
    sound_free(&door_sound);
    sound_free(&whip_sound);
    for (int g = 0; g < GUN_COUNT; g++) { sound_free(&gun_sounds[g]); sound_free(&reload_sounds[g]); }
    Mix_CloseAudio();
    texture_free(&shotgun_tex);
    texture_free(&weapon_kit_tex);
    texture_free(&pistol_tex);
    for (int t = ENEMY_TYPE_COUNT - 1; t >= 0; t--) {
        for (int d = 7; d >= 0; d--) { texture_free(&enemy_tex[t][d]); }
    }
    texture_free(&exit_tex);
    texture_free(&door_tex);
    for (int wl = 2; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
    map_free(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
