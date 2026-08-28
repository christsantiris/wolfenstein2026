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
#include "systems/save_load.h"
#include "ui/slot_picker.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define LEVEL_COUNT 10

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
    int saved_difficulty = game->difficulty;
    int saved_has_weapon[GUN_COUNT];
    int saved_ammo_per_gun[GUN_COUNT];
    int saved_reserve_ammo_per_gun[GUN_COUNT];
    WeaponDef saved_weapon = game->current_weapon;
    for (int i = 0; i < GUN_COUNT; i++) {
        saved_has_weapon[i] = game->has_weapon[i];
        saved_ammo_per_gun[i] = game->ammo_per_gun[i];
        saved_reserve_ammo_per_gun[i] = game->reserve_ammo_per_gun[i];
    }
    game_init(game);
    game->score = saved_score;
    game->difficulty = saved_difficulty;
    for (int i = 0; i < GUN_COUNT; i++) {
        game->has_weapon[i] = saved_has_weapon[i];
        game->ammo_per_gun[i] = saved_ammo_per_gun[i];
        game->reserve_ammo_per_gun[i] = saved_reserve_ammo_per_gun[i];
    }
    game->current_weapon = saved_weapon;
    game->ammo = game->ammo_per_gun[saved_weapon.type];
    enemy_list_init(&game->enemies, map, level, game->difficulty, player->x, player->y);
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

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, SCREEN_W, SCREEN_H);

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

    Texture wall_tex[LEVEL_COUNT] = { 0 };
    const char *wall_paths[LEVEL_COUNT] = {
        "assets/textures/wall_level1_brick.ppm",
        "assets/textures/wall_level2_stone.ppm",
        "assets/textures/wall_level3_sandstone.ppm",
        "assets/textures/wall_level4_blue_brick.ppm",
        "assets/textures/wall_level5_wood.ppm",
        "assets/textures/wall_level6_moss_stone.ppm",
        "assets/textures/wall_level7_military_brick.ppm",
        "assets/textures/wall_level8_steel.ppm",
        "assets/textures/wall_level9_bunker.ppm",
        "assets/textures/wall_level10_obsidian.ppm"
    };
    void (*wall_fallbacks[LEVEL_COUNT])(Texture *) = {
        texture_generate_brick,
        texture_generate_stone,
        texture_generate_sandstone,
        texture_generate_blue_brick,
        texture_generate_wood,
        texture_generate_moss_stone,
        texture_generate_red_blue_brick,
        texture_generate_metal_panels,
        texture_generate_command_bunker,
        texture_generate_obsidian_command
    };
    int wall_tex_ready = 1;
    for (int wl = 0; wl < LEVEL_COUNT; wl++) {
        if (texture_load_ppm(&wall_tex[wl], wall_paths[wl]) != 0) {
            if (texture_create(&wall_tex[wl], 64, 64) != 0) {
                wall_tex_ready = 0;
                break;
            }
            wall_fallbacks[wl](&wall_tex[wl]);
        }
    }
    if (!wall_tex_ready) {
        for (int wl = 0; wl < LEVEL_COUNT; wl++) { texture_free(&wall_tex[wl]); }
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Texture floor_tex[LEVEL_COUNT] = { 0 };
    const char *floor_paths[LEVEL_COUNT] = {
        "assets/textures/floor_level1_flagstone.ppm",
        "assets/textures/floor_level2_stone.ppm",
        "assets/textures/floor_level3_sandstone.ppm",
        "assets/textures/floor_level4_blue_slate.ppm",
        "assets/textures/floor_level5_wood.ppm",
        "assets/textures/floor_level6_moss_stone.ppm",
        "assets/textures/floor_level7_military_brick.ppm",
        "assets/textures/floor_level8_steel.ppm",
        "assets/textures/floor_level9_bunker.ppm",
        "assets/textures/floor_level10_obsidian.ppm"
    };
    int floor_tex_ready = 1;
    for (int fl = 0; fl < LEVEL_COUNT; fl++) {
        if (texture_load_ppm(&floor_tex[fl], floor_paths[fl]) != 0) {
            if (texture_create(&floor_tex[fl], 64, 64) != 0) {
                floor_tex_ready = 0;
                break;
            }
            wall_fallbacks[fl](&floor_tex[fl]);
        }
    }
    if (!floor_tex_ready) {
        for (int fl = 0; fl < LEVEL_COUNT; fl++) { texture_free(&floor_tex[fl]); }
        for (int wl = 0; wl < LEVEL_COUNT; wl++) { texture_free(&wall_tex[wl]); }
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Texture door_tex;
    if (texture_create(&door_tex, 64, 64) != 0) {
        for (int fl = LEVEL_COUNT - 1; fl >= 0; fl--) { texture_free(&floor_tex[fl]); }
        for (int wl = LEVEL_COUNT - 1; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
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
        for (int fl = LEVEL_COUNT - 1; fl >= 0; fl--) { texture_free(&floor_tex[fl]); }
        for (int wl = LEVEL_COUNT - 1; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
        map_free(&map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    texture_generate_exit_door(&exit_tex);

    Texture ammo_pickup_tex;
    if (texture_load_ppm(&ammo_pickup_tex, "assets/sprites/ammo_pickup.ppm") != 0) {
        texture_create(&ammo_pickup_tex, 64, 64);
        texture_generate_ammo_pickup(&ammo_pickup_tex);
    }

    Texture health_pickup_tex;
    if (texture_load_ppm(&health_pickup_tex, "assets/sprites/health_pickup.ppm") != 0) {
        texture_create(&health_pickup_tex, 64, 64);
        texture_generate_health_pickup(&health_pickup_tex);
    }

    Texture face_tex[HUD_FACE_COUNT] = { 0 };
    const char *face_paths[HUD_FACE_COUNT] = {
        [HUD_FACE_HEALTHY] = "assets/sprites/player_face_healthy.ppm",
        [HUD_FACE_WOUNDED] = "assets/sprites/player_face_wounded.ppm",
        [HUD_FACE_INJURED] = "assets/sprites/player_face_injured.ppm",
        [HUD_FACE_CRITICAL] = "assets/sprites/player_face_critical.ppm",
        [HUD_FACE_DEAD] = "assets/sprites/player_face_dead.ppm"
    };
    for (int f = 0; f < HUD_FACE_COUNT; f++) {
        texture_load_ppm(&face_tex[f], face_paths[f]);
    }

    Texture weapon_kit_tex;
    texture_create(&weapon_kit_tex, 64, 64);
    texture_generate_weapon_kit(&weapon_kit_tex);

    Texture weapon_kit_ak47_tex;
    texture_create(&weapon_kit_ak47_tex, 64, 64);
    texture_generate_weapon_kit_ak47(&weapon_kit_ak47_tex);

    Texture weapon_kit_dual_tex;
    texture_create(&weapon_kit_dual_tex, 64, 64);
    texture_generate_weapon_kit_dual(&weapon_kit_dual_tex);

    Texture weapon_kit_battle_rifle_tex;
    texture_create(&weapon_kit_battle_rifle_tex, 64, 64);
    texture_generate_weapon_kit_battle_rifle(&weapon_kit_battle_rifle_tex);

    Texture shotgun_tex;
    if (texture_load_ppm(&shotgun_tex, "assets/sprites/shotgun.ppm") != 0) {
        texture_create(&shotgun_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { shotgun_tex.pixels[i] = 0; }
    }

    Texture pistol_tex;
    if (texture_load_ppm(&pistol_tex, "assets/sprites/pistol.ppm") != 0) {
        texture_create(&pistol_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { pistol_tex.pixels[i] = 0; }
    }

    Texture knife_tex;
    if (texture_load_ppm(&knife_tex, "assets/sprites/knife.ppm") != 0) {
        texture_create(&knife_tex, 64, 64);
        for (int i = 0; i < 64 * 64; i++) {
            knife_tex.pixels[i * 3] = 255;
            knife_tex.pixels[i * 3 + 1] = 0;
            knife_tex.pixels[i * 3 + 2] = 255;
        }
    }

    Texture dual_handgun_tex;
    if (texture_load_ppm(&dual_handgun_tex, "assets/sprites/dual_handgun.ppm") != 0) {
        texture_create(&dual_handgun_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { dual_handgun_tex.pixels[i] = 0; }
    }

    Texture ak47_tex;
    if (texture_load_ppm(&ak47_tex, "assets/sprites/ak47.ppm") != 0) {
        texture_create(&ak47_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { ak47_tex.pixels[i] = 0; }
    }

    Texture battle_rifle_tex;
    if (texture_load_ppm(&battle_rifle_tex, "assets/sprites/battle_rifle.ppm") != 0) {
        texture_create(&battle_rifle_tex, 64, 64);
        for (int i = 0; i < 64 * 64 * 3; i++) { battle_rifle_tex.pixels[i] = 0; }
    }

    const Texture *weapon_textures[GUN_COUNT] = {
        [GUN_9MM_HANDGUN] = &pistol_tex,
        [GUN_DUAL_HANDGUN] = &dual_handgun_tex,
        [GUN_SHOTGUN] = &shotgun_tex,
        [GUN_AK47] = &ak47_tex,
        [GUN_BATTLE_RIFLE] = &battle_rifle_tex,
        [GUN_KNIFE] = &knife_tex
    };
    Texture reload_tex[GUN_COUNT][WEAPON_RELOAD_FRAME_COUNT] = { 0 };
    const char *reload_paths[GUN_COUNT][WEAPON_RELOAD_FRAME_COUNT] = {
        [GUN_9MM_HANDGUN] = { "assets/sprites/pistol_reload_1.ppm", "assets/sprites/pistol_reload_2.ppm", "assets/sprites/pistol_reload_3.ppm" },
        [GUN_DUAL_HANDGUN] = { "assets/sprites/dual_handgun_reload_1.ppm", "assets/sprites/dual_handgun_reload_2.ppm", "assets/sprites/dual_handgun_reload_3.ppm" },
        [GUN_SHOTGUN] = { "assets/sprites/shotgun_reload_1.ppm", "assets/sprites/shotgun_reload_2.ppm", "assets/sprites/shotgun_reload_3.ppm" },
        [GUN_AK47] = { "assets/sprites/ak47_reload_1.ppm", "assets/sprites/ak47_reload_2.ppm", "assets/sprites/ak47_reload_3.ppm" },
        [GUN_BATTLE_RIFLE] = { "assets/sprites/battle_rifle_reload_1.ppm", "assets/sprites/battle_rifle_reload_2.ppm", "assets/sprites/battle_rifle_reload_3.ppm" }
    };
    for (int gun = 0; gun < GUN_KNIFE; gun++) {
        const Texture *base_tex = weapon_textures[gun];
        for (int frame = 0; frame < WEAPON_RELOAD_FRAME_COUNT; frame++) {
            if (texture_load_ppm(&reload_tex[gun][frame], reload_paths[gun][frame]) != 0) {
                texture_create(&reload_tex[gun][frame], base_tex->width, base_tex->height);
                for (int i = 0; i < base_tex->width * base_tex->height * 3; i++) {
                    reload_tex[gun][frame].pixels[i] = base_tex->pixels[i];
                }
            }
        }
    }

    Texture enemy_tex[ENEMY_TYPE_COUNT][ENEMY_SPRITE_FRAMES];
    int etw = 64, eth = 64;

    /* Build directional frames from each enemy type's two walking poses. */
#define BUILD_ENEMY_FRAMES(T) \
    do { \
        for (int _d = 0; _d < 8; _d++) { \
            if (_d == 4) { continue; } \
            texture_create(&enemy_tex[T][_d], etw, eth); \
        } \
        texture_derive_guard_dirs(&enemy_tex[T][0]); \
        for (int _d = 0; _d < 8; _d++) { \
            if (_d == 4) { continue; } \
            texture_create(&enemy_tex[T][ENEMY_SPRITE_WALK_B + _d], etw, eth); \
        } \
        texture_derive_guard_dirs(&enemy_tex[T][ENEMY_SPRITE_WALK_B]); \
    } while (0)

    const char *enemy_paths[ENEMY_TYPE_COUNT] = {
        "assets/sprites/guard_front.ppm",
        "assets/sprites/officer_front.ppm",
        "assets/sprites/ss_front.ppm",
        "assets/sprites/boss_front.ppm",
        "assets/sprites/shotgun_guard_front.ppm"
    };
    const char *enemy_walk_paths[ENEMY_TYPE_COUNT] = {
        "assets/sprites/guard_walk.ppm",
        "assets/sprites/officer_walk.ppm",
        "assets/sprites/ss_walk.ppm",
        "assets/sprites/boss_walk.ppm",
        "assets/sprites/shotgun_guard_walk.ppm"
    };
    const char *enemy_aim_paths[ENEMY_TYPE_COUNT] = {
        "assets/sprites/guard_aim.ppm",
        "assets/sprites/officer_aim.ppm",
        "assets/sprites/ss_aim.ppm",
        "assets/sprites/boss_aim.ppm",
        "assets/sprites/shotgun_guard_aim.ppm"
    };
    const char *enemy_corpse_paths[ENEMY_TYPE_COUNT] = {
        "assets/sprites/guard_corpse.ppm",
        "assets/sprites/officer_corpse.ppm",
        "assets/sprites/ss_corpse.ppm",
        "assets/sprites/boss_corpse.ppm",
        "assets/sprites/shotgun_guard_corpse.ppm"
    };
    const int enemy_muzzle_y[ENEMY_TYPE_COUNT] = { 14, 13, 14, 15, 14 };
    const int enemy_flash_radius[ENEMY_TYPE_COUNT] = { 3, 3, 4, 5, 5 };
    for (int t = 0; t < ENEMY_TYPE_COUNT; t++) {
        if (texture_load_ppm(&enemy_tex[t][4], enemy_paths[t]) != 0) {
            texture_create(&enemy_tex[t][4], etw, eth);
            if (t == ENEMY_TYPE_BOSS) {
                texture_generate_boss_dir(&enemy_tex[t][4], 4);
            } else {
                texture_generate_guard_dir(&enemy_tex[t][4], 4);
            }
        }
        if (texture_load_ppm(&enemy_tex[t][ENEMY_SPRITE_WALK_B + 4], enemy_walk_paths[t]) != 0) {
            texture_create(&enemy_tex[t][ENEMY_SPRITE_WALK_B + 4], etw, eth);
            memcpy(enemy_tex[t][ENEMY_SPRITE_WALK_B + 4].pixels, enemy_tex[t][4].pixels, (size_t)etw * eth * 3);
            texture_generate_guard_walk_b(&enemy_tex[t][ENEMY_SPRITE_WALK_B + 4], 4);
        }
        if (texture_load_ppm(&enemy_tex[t][ENEMY_SPRITE_AIM], enemy_aim_paths[t]) != 0) {
            texture_create(&enemy_tex[t][ENEMY_SPRITE_AIM], etw, eth);
            memcpy(enemy_tex[t][ENEMY_SPRITE_AIM].pixels, enemy_tex[t][4].pixels, (size_t)etw * eth * 3);
        }
        if (texture_load_ppm(&enemy_tex[t][ENEMY_SPRITE_CORPSE], enemy_corpse_paths[t]) != 0) {
            texture_create(&enemy_tex[t][ENEMY_SPRITE_CORPSE], etw, eth);
            memcpy(enemy_tex[t][ENEMY_SPRITE_CORPSE].pixels, enemy_tex[t][4].pixels, (size_t)etw * eth * 3);
        }
        texture_create(&enemy_tex[t][ENEMY_SPRITE_FIRE], etw, eth);
        memcpy(enemy_tex[t][ENEMY_SPRITE_FIRE].pixels, enemy_tex[t][ENEMY_SPRITE_AIM].pixels, (size_t)etw * eth * 3);
        texture_generate_guard_walk_b(&enemy_tex[t][ENEMY_SPRITE_FIRE], 4);
        texture_generate_muzzle_flash(&enemy_tex[t][ENEMY_SPRITE_FIRE], etw / 2, enemy_muzzle_y[t] - 2, enemy_flash_radius[t]);
        BUILD_ENEMY_FRAMES(t);
    }

#undef BUILD_ENEMY_FRAMES

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

    Music level_music[LEVEL_COUNT] = { 0 };
    music_load(&level_music[0], "assets/music/level1theme.mp3");
    music_load(&level_music[1], "assets/music/level2theme.mp3");
    music_load(&level_music[2], "assets/music/level3theme.mp3");
    music_load(&level_music[3], "assets/music/level4theme.mp3");
    music_load(&level_music[4], "assets/music/level5theme.mp3");
    music_load(&level_music[5], "assets/music/level6theme.mp3");
    music_load(&level_music[6], "assets/music/level7theme.mp3");
    music_load(&level_music[7], "assets/music/level8theme.mp3");
    music_load(&level_music[8], "assets/music/level9theme.mp3");
    music_load(&level_music[9], "assets/music/level10theme.mp3");

    HighScoreTable hs_table;
    highscore_load(&hs_table);
    int hs_rank = 0;

    AppState app_state = APP_LANDING;
    Menu menu = { 0 };
    menu.music_on = 1;
    menu.sound_on = 1;
    menu.minimap_on = 1;
    menu.enemy_markers_on = 0;
    int current_level = 1;
#ifdef DEBUG_START_LEVEL
    current_level = DEBUG_START_LEVEL;
    game.difficulty = 0;
    start_game(&map, &player, &game, current_level);
    if (current_level - 1 < LEVEL_COUNT) { music_play(&level_music[current_level - 1]); }
    app_state = APP_PLAYING;
#endif
    int game_over = 0;
    int game_won = 0;
    SlotPicker slot_picker = { 0 };
    int running = 1;
    float total_time = 0.0f;
    SDL_Event e;
    Uint32 last_ticks = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;
        total_time += dt;

        int w = SCREEN_W, h = SCREEN_H;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f) {
                Uint32 flags = SDL_GetWindowFlags(window);
                if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(window, 0);
                } else {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            }

            if (slot_picker.is_open) {
                int picked_slot = 0;
                SlotResult sr = slot_picker_handle_event(&slot_picker, &e, w, h, &picked_slot);
                if (sr == SLOT_RESULT_SELECTED) {
                    SaveSettings settings = {
                        .music_on = menu.music_on,
                        .sound_on = menu.sound_on,
                        .minimap_on = menu.minimap_on,
                        .enemy_positions_on = menu.enemy_markers_on
                    };
                    if (slot_picker.is_save) {
                        save_game(picked_slot, current_level, &player, &game, &map, &settings);
                    } else {
                        if (load_game(picked_slot, &current_level, &player, &game, &map, &settings) == 0) {
                            menu.music_on = settings.music_on;
                            menu.sound_on = settings.sound_on;
                            menu.minimap_on = settings.minimap_on;
                            menu.enemy_markers_on = settings.enemy_positions_on;
                            sound_set_enabled(menu.sound_on);
                            game_over = 0;
                            game_won = 0;
                            menu.is_open = 0;
                            app_state = APP_PLAYING;
                            music_stop();
                            int idx = current_level - 1;
                            if (idx >= 0 && idx < LEVEL_COUNT) {
                                music_play(&level_music[idx]);
                                if (!menu.music_on) {
                                    Mix_PauseMusic();
                                }
                            }
                        }
                    }
                }
                continue;
            }

            if (app_state == APP_LANDING) {
                LandingResult lr = landing_handle_event(&e, w, h);
                if (lr == LANDING_NEW_GAME) {
                    difficulty_screen_reset();
                    app_state = APP_DIFFICULTY;
                } else if (lr == LANDING_LOAD) {
                    slot_picker_open(&slot_picker, 0);
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
                    game.difficulty = (int)d;
                    menu.difficulty = (int)d;
                    start_game(&map, &player, &game, current_level);
                    music_play(&level_music[current_level - 1]);
                    game_over = 0;
                    game_won = 0;
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
                        game_over = 0;
                        game_won = 0;
                    } else if (action == MENU_ACTION_MUSIC_TOGGLE) {
                        if (menu.music_on) { Mix_ResumeMusic(); } else { Mix_PauseMusic(); }
                    } else if (action == MENU_ACTION_SOUND_TOGGLE) {
                        sound_set_enabled(menu.sound_on);
                    } else if (action == MENU_ACTION_SAVE) {
                        slot_picker_open(&slot_picker, 1);
                    } else if (action == MENU_ACTION_LOAD) {
                        slot_picker_open(&slot_picker, 0);
                    }
                } else if (!game_over && !game_won) {
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
                        menu.minimap_on = !menu.minimap_on;
                    }
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_n) {
                        menu.enemy_markers_on = !menu.enemy_markers_on;
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
                } else if (game_over || game_won) {
                    GameOverResult result = game_over_handle_event(&e, w, h, hs_table.count);
                    if (result == GAME_OVER_QUIT) {
                        running = 0;
                    } else if (result == GAME_OVER_NEW_GAME) {
                        music_stop();
                        landing_reset();
                        app_state = APP_LANDING;
                        game_over = 0;
                        game_won = 0;
                    } else if (result == GAME_OVER_LOAD) {
                        slot_picker_open(&slot_picker, 0);
                    }
                }
            }
        }

        if (app_state == APP_PLAYING && !menu.is_open && !game_over && !game_won) {
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
                        if (game.current_weapon.type == GUN_KNIFE) {
                            continue;
                        }
                        game.reserve_ammo_per_gun[game.current_weapon.type] += AMMO_PICKUP_AMOUNT;
                        if (game.reserve_ammo_per_gun[game.current_weapon.type] > AMMO_RESERVE_MAX) {
                            game.reserve_ammo_per_gun[game.current_weapon.type] = AMMO_RESERVE_MAX;
                        }
                    } else if (it->type == ITEM_HEALTH) {
                        static const int HEALTH_PICKUP[4] = { 40, 25, 15, 10 };
                        int heal = HEALTH_PICKUP[game.difficulty < 4 ? game.difficulty : 3];
                        game.health += heal;
                        if (game.health > 100) { game.health = 100; }
                    } else if (it->type == ITEM_WEAPON_KIT_DUAL) {
                        if (!game.has_weapon[GUN_DUAL_HANDGUN]) {
                            game.has_weapon[GUN_DUAL_HANDGUN] = 1;
                            game.ammo_per_gun[GUN_DUAL_HANDGUN] = weapon_def(GUN_DUAL_HANDGUN)->max_ammo;
                            game.reserve_ammo_per_gun[GUN_DUAL_HANDGUN] = AMMO_RESERVE_MAX;
                            game.ammo_per_gun[game.current_weapon.type] = game.ammo;
                            game.current_weapon = *weapon_def(GUN_DUAL_HANDGUN);
                            game.ammo = game.ammo_per_gun[GUN_DUAL_HANDGUN];
                            game.is_reloading = 0;
                            game.reload_timer = 0.0f;
                            game.shot_cooldown = 0.0f;
                        }
                    } else if (it->type == ITEM_WEAPON_KIT) {
                        if (!game.has_weapon[GUN_SHOTGUN]) {
                            game.has_weapon[GUN_SHOTGUN] = 1;
                            game.ammo_per_gun[GUN_SHOTGUN] = weapon_def(GUN_SHOTGUN)->max_ammo;
                            game.reserve_ammo_per_gun[GUN_SHOTGUN] = AMMO_RESERVE_MAX;
                            game.ammo_per_gun[game.current_weapon.type] = game.ammo;
                            game.current_weapon = *weapon_def(GUN_SHOTGUN);
                            game.ammo = game.ammo_per_gun[GUN_SHOTGUN];
                            game.is_reloading = 0;
                            game.reload_timer = 0.0f;
                            game.shot_cooldown = 0.0f;
                        }
                    } else if (it->type == ITEM_WEAPON_KIT_AK47) {
                        if (!game.has_weapon[GUN_AK47]) {
                            game.has_weapon[GUN_AK47] = 1;
                            game.ammo_per_gun[GUN_AK47] = weapon_def(GUN_AK47)->max_ammo;
                            game.reserve_ammo_per_gun[GUN_AK47] = AMMO_RESERVE_MAX;
                            game.ammo_per_gun[game.current_weapon.type] = game.ammo;
                            game.current_weapon = *weapon_def(GUN_AK47);
                            game.ammo = game.ammo_per_gun[GUN_AK47];
                            game.is_reloading = 0;
                            game.reload_timer = 0.0f;
                            game.shot_cooldown = 0.0f;
                        }
                    } else if (it->type == ITEM_WEAPON_KIT_BATTLE_RIFLE) {
                        if (!game.has_weapon[GUN_BATTLE_RIFLE]) {
                            game.has_weapon[GUN_BATTLE_RIFLE] = 1;
                            game.ammo_per_gun[GUN_BATTLE_RIFLE] = weapon_def(GUN_BATTLE_RIFLE)->max_ammo;
                            game.reserve_ammo_per_gun[GUN_BATTLE_RIFLE] = AMMO_RESERVE_MAX;
                            game.ammo_per_gun[game.current_weapon.type] = game.ammo;
                            game.current_weapon = *weapon_def(GUN_BATTLE_RIFLE);
                            game.ammo = game.ammo_per_gun[GUN_BATTLE_RIFLE];
                            game.is_reloading = 0;
                            game.reload_timer = 0.0f;
                            game.shot_cooldown = 0.0f;
                        }
                    }
                    it->active = 0;
                }
            }
            if (enemy_list_all_dead(&game.enemies)) {
                map_unlock_exits(&map);
            }
            if (map_cell(&map, (int)player.x, (int)player.y) == MAP_CELL_EXIT_OPEN) {
                if (current_level >= LEVEL_COUNT && enemy_list_all_dead(&game.enemies)) {
                    game_won = 1;
                    hs_rank = highscore_insert(&hs_table, game.score);
                    highscore_save(&hs_table);
                } else {
                    current_level++;
                    if (start_game(&map, &player, &game, current_level) != 0) {
                        current_level = 1;
                        music_stop();
                        landing_reset();
                        app_state = APP_LANDING;
                    } else {
                        sound_play(&level_sound);
                        int idx = current_level - 1;
                        if (menu.music_on && idx >= 0 && idx < LEVEL_COUNT) {
                            music_play(&level_music[idx]);
                        }
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
            int wall_idx = (current_level - 1 < LEVEL_COUNT) ? current_level - 1 : LEVEL_COUNT - 1;
            raycaster_render(renderer, &map, &player, &wall_tex[wall_idx], &floor_tex[wall_idx], &door_tex, &exit_tex, zbuf, w, h - HUD_HEIGHT, total_time);
            sprite_render_all(renderer, &player, &game.enemies, zbuf, enemy_tex, w, h - HUD_HEIGHT);
            item_render_all(renderer, &player, &game.items, zbuf, &ammo_pickup_tex, &health_pickup_tex, &weapon_kit_tex, &weapon_kit_ak47_tex, &weapon_kit_dual_tex, &weapon_kit_battle_rifle_tex, w, h - HUD_HEIGHT);
            int knife_visible = game.current_weapon.type == GUN_KNIFE || game.pistol_whip_timer > 0.0f;
            const Texture *weapon_tex = knife_visible ? &knife_tex : weapon_textures[game.current_weapon.type];
            float shot_timer = knife_visible ? 0.0f : game.shot_timer;
            weapon_render(renderer, &game.current_weapon, weapon_tex, reload_tex[game.current_weapon.type], game.ammo, shot_timer, game.shot_cooldown, game.pistol_whip_timer, game.is_reloading, game.reload_timer, w, h - HUD_HEIGHT);
            if (menu.minimap_on) {
                minimap_render(renderer, &map, &player, &game.enemies, menu.enemy_markers_on);
            }
            int enemies_remaining = 0;
            for (int i = 0; i < game.enemies.count; i++) {
                if (game.enemies.enemies[i].active) {
                    enemies_remaining++;
                }
            }
            hud_render(renderer, w, h, game.health, game.ammo, game.reserve_ammo_per_gun[game.current_weapon.type], game.score, enemies_remaining, face_tex);
            if (game.level_clear_timer > 0.0f) {
                hud_draw_level_clear(renderer, w, h - HUD_HEIGHT, game.level_clear_timer);
            } else if (enemy_list_all_dead(&game.enemies)) {
                hud_draw_exit_open(renderer, w, h - HUD_HEIGHT);
            }
            if (game.hit_flash_timer > 0.0f) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 180, 0, 0, 100);
                SDL_Rect flash = { 0, 0, w, h };
                SDL_RenderFillRect(renderer, &flash);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            if (menu.is_open) {
                menu.current_level = current_level;
                menu_render(renderer, &menu, w, h);
            }
            if (game_over) {
                game_over_render(renderer, w, h, game.score, hs_rank, &hs_table);
            } else if (game_won) {
                victory_render(renderer, w, h, game.score, hs_rank, &hs_table);
            }
        }

        if (slot_picker.is_open) {
            slot_picker_render(renderer, &slot_picker, w, h);
        }

        SDL_RenderPresent(renderer);
    }

    free(zbuf);
    music_stop();
    for (int m = 0; m < LEVEL_COUNT; m++) { music_free(&level_music[m]); }
    sound_free(&enemy_sound);
    sound_free(&level_sound);
    sound_free(&door_sound);
    sound_free(&whip_sound);
    for (int g = 0; g < GUN_COUNT; g++) { sound_free(&gun_sounds[g]); sound_free(&reload_sounds[g]); }
    Mix_CloseAudio();
    for (int gun = GUN_KNIFE - 1; gun >= 0; gun--) {
        for (int frame = WEAPON_RELOAD_FRAME_COUNT - 1; frame >= 0; frame--) { texture_free(&reload_tex[gun][frame]); }
    }
    texture_free(&battle_rifle_tex);
    texture_free(&ak47_tex);
    texture_free(&shotgun_tex);
    texture_free(&dual_handgun_tex);
    texture_free(&knife_tex);
    texture_free(&weapon_kit_tex);
    texture_free(&weapon_kit_ak47_tex);
    texture_free(&weapon_kit_dual_tex);
    texture_free(&weapon_kit_battle_rifle_tex);
    texture_free(&pistol_tex);
    for (int f = HUD_FACE_COUNT - 1; f >= 0; f--) { texture_free(&face_tex[f]); }
    for (int t = ENEMY_TYPE_COUNT - 1; t >= 0; t--) {
        for (int d = ENEMY_SPRITE_FRAMES - 1; d >= 0; d--) { texture_free(&enemy_tex[t][d]); }
    }
    texture_free(&exit_tex);
    texture_free(&door_tex);
    for (int fl = LEVEL_COUNT - 1; fl >= 0; fl--) { texture_free(&floor_tex[fl]); }
    for (int wl = LEVEL_COUNT - 1; wl >= 0; wl--) { texture_free(&wall_tex[wl]); }
    map_free(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
