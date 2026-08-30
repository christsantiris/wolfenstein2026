#include "systems/save_load.h"
#include "core/player.h"
#include "core/game.h"
#include "core/map.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define SAVE_MAGIC   "WOLF2026"
#define SAVE_VERSION 14
#define SAVE_GUN_COUNT_V7 5
#define SAVE_GUN_COUNT_V10 6

static void create_save_directory(void) {
#ifdef _WIN32
    _mkdir("saves");
#else
    mkdir("saves", 0755);
#endif
}

static void save_path(int slot, char *buf, int bufsz) {
    snprintf(buf, bufsz, "saves/slot%d.sav", slot);
}

int save_slot_exists(int slot) {
    char path[64];
    save_path(slot, path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

int save_game(int slot, int level, const Player *p, const GameState *g, const Map *m, const SaveSettings *s) {
    create_save_directory();
    char path[64];
    save_path(slot, path, sizeof(path));

    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }

    fwrite(SAVE_MAGIC, 8, 1, f);
    uint32_t ver = SAVE_VERSION;
    fwrite(&ver, sizeof(ver), 1, f);

    fwrite(&level, sizeof(level), 1, f);

    fwrite(&p->x, sizeof(p->x), 1, f);
    fwrite(&p->y, sizeof(p->y), 1, f);
    fwrite(&p->angle, sizeof(p->angle), 1, f);

    fwrite(&g->difficulty, sizeof(g->difficulty), 1, f);
    fwrite(&g->health, sizeof(g->health), 1, f);
    fwrite(&g->ammo, sizeof(g->ammo), 1, f);
    fwrite(g->reserve_ammo_per_gun, sizeof(g->reserve_ammo_per_gun), 1, f);
    fwrite(&g->score, sizeof(g->score), 1, f);
    int wtype = (int)g->current_weapon.type;
    fwrite(&wtype, sizeof(wtype), 1, f);
    fwrite(g->has_weapon, sizeof(g->has_weapon), 1, f);
    fwrite(g->ammo_per_gun, sizeof(g->ammo_per_gun), 1, f);
    fwrite(&g->grenade.active, sizeof(g->grenade.active), 1, f);
    fwrite(&g->grenade.x, sizeof(g->grenade.x), 1, f);
    fwrite(&g->grenade.y, sizeof(g->grenade.y), 1, f);
    fwrite(&g->grenade.dir_x, sizeof(g->grenade.dir_x), 1, f);
    fwrite(&g->grenade.dir_y, sizeof(g->grenade.dir_y), 1, f);
    fwrite(&g->grenade.distance, sizeof(g->grenade.distance), 1, f);
    fwrite(&g->grenade.explosion_x, sizeof(g->grenade.explosion_x), 1, f);
    fwrite(&g->grenade.explosion_y, sizeof(g->grenade.explosion_y), 1, f);
    fwrite(&g->grenade.explosion_timer, sizeof(g->grenade.explosion_timer), 1, f);

    fwrite(&g->enemies.count, sizeof(g->enemies.count), 1, f);
    for (int i = 0; i < g->enemies.count; i++) {
        const Enemy *e = &g->enemies.enemies[i];
        fwrite(&e->x, sizeof(e->x), 1, f);
        fwrite(&e->y, sizeof(e->y), 1, f);
        fwrite(&e->angle, sizeof(e->angle), 1, f);
        fwrite(&e->health, sizeof(e->health), 1, f);
        fwrite(&e->active, sizeof(e->active), 1, f);
        int estate = (int)e->state;
        fwrite(&estate, sizeof(estate), 1, f);
        fwrite(&e->attack_timer, sizeof(e->attack_timer), 1, f);
        fwrite(&e->attack_flash_timer, sizeof(e->attack_flash_timer), 1, f);
        int etype = (int)e->type;
        fwrite(&etype, sizeof(etype), 1, f);
        fwrite(&e->walk_frame, sizeof(e->walk_frame), 1, f);
        fwrite(&e->walk_timer, sizeof(e->walk_timer), 1, f);
        fwrite(&e->reinforcements_called, sizeof(e->reinforcements_called), 1, f);
    }

    fwrite(&g->items.count, sizeof(g->items.count), 1, f);
    for (int i = 0; i < g->items.count; i++) {
        const Item *it = &g->items.items[i];
        fwrite(&it->x, sizeof(it->x), 1, f);
        fwrite(&it->y, sizeof(it->y), 1, f);
        int itype = (int)it->type;
        fwrite(&itype, sizeof(itype), 1, f);
        fwrite(&it->active, sizeof(it->active), 1, f);
    }

    fwrite(&s->music_on, sizeof(s->music_on), 1, f);
    fwrite(&s->sound_on, sizeof(s->sound_on), 1, f);
    fwrite(&s->minimap_on, sizeof(s->minimap_on), 1, f);
    fwrite(&s->enemy_positions_on, sizeof(s->enemy_positions_on), 1, f);
    fwrite(&s->weapon_pickups_on, sizeof(s->weapon_pickups_on), 1, f);
    fwrite(&s->enemy_health_bars_on, sizeof(s->enemy_health_bars_on), 1, f);

    fwrite(&m->width, sizeof(m->width), 1, f);
    fwrite(&m->height, sizeof(m->height), 1, f);
    fwrite(m->cells, sizeof(int) * m->width * m->height, 1, f);

    fclose(f);
    return 0;
}

int load_game(int slot, int *level, Player *p, GameState *g, Map *m, SaveSettings *s) {
    char path[64];
    save_path(slot, path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }

    char magic[8];
    if (fread(magic, 8, 1, f) != 1 || memcmp(magic, SAVE_MAGIC, 8) != 0) {
        fclose(f);
        return -1;
    }
    uint32_t ver;
    if (fread(&ver, sizeof(ver), 1, f) != 1 || ver < 5 || ver > SAVE_VERSION) {
        fclose(f);
        return -1;
    }

    SaveSettings loaded_settings = *s;
    if (ver < 13) {
        loaded_settings.weapon_pickups_on = 0;
    }
    if (ver < 14) {
        loaded_settings.enemy_health_bars_on = 1;
    }

    if (fread(level, sizeof(*level), 1, f) != 1) {
        fclose(f);
        return -1;
    }
    if (ver < 9 && *level >= 6) {
        (*level)++;
    }
    if (ver < 10 && *level >= 8) {
        (*level)++;
    }

    char map_path[64];
    snprintf(map_path, sizeof(map_path), "assets/maps/level%d.map", *level);
    map_free(m);
    if (map_load(m, map_path) != 0) {
        fclose(f);
        return -1;
    }

    float px, py, pa;
    if (fread(&px, sizeof(px), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&py, sizeof(py), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&pa, sizeof(pa), 1, f) != 1) { fclose(f); return -1; }
    player_init(p, px, py, pa);

    game_init(g);
    if (fread(&g->difficulty, sizeof(g->difficulty), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&g->health, sizeof(g->health), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&g->ammo, sizeof(g->ammo), 1, f) != 1) { fclose(f); return -1; }
    size_t saved_gun_bytes;
    if (ver >= 11) {
        saved_gun_bytes = sizeof(g->reserve_ammo_per_gun);
    } else if (ver >= 8) {
        saved_gun_bytes = sizeof(int) * SAVE_GUN_COUNT_V10;
    } else {
        saved_gun_bytes = sizeof(int) * SAVE_GUN_COUNT_V7;
    }
    if (fread(g->reserve_ammo_per_gun, saved_gun_bytes, 1, f) != 1) { fclose(f); return -1; }
    if (fread(&g->score, sizeof(g->score), 1, f) != 1) { fclose(f); return -1; }
    int wtype;
    if (fread(&wtype, sizeof(wtype), 1, f) != 1) { fclose(f); return -1; }
    g->current_weapon = *weapon_def((GunType)wtype);
    if (fread(g->has_weapon, saved_gun_bytes, 1, f) != 1) { fclose(f); return -1; }
    if (fread(g->ammo_per_gun, saved_gun_bytes, 1, f) != 1) { fclose(f); return -1; }
    if (ver >= 12) {
        if (fread(&g->grenade.active, sizeof(g->grenade.active), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.x, sizeof(g->grenade.x), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.y, sizeof(g->grenade.y), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.dir_x, sizeof(g->grenade.dir_x), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.dir_y, sizeof(g->grenade.dir_y), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.distance, sizeof(g->grenade.distance), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.explosion_x, sizeof(g->grenade.explosion_x), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.explosion_y, sizeof(g->grenade.explosion_y), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&g->grenade.explosion_timer, sizeof(g->grenade.explosion_timer), 1, f) != 1) { fclose(f); return -1; }
    }

    if (fread(&g->enemies.count, sizeof(g->enemies.count), 1, f) != 1) { fclose(f); return -1; }
    if (g->enemies.count < 0 || g->enemies.count > MAX_ENEMIES) { fclose(f); return -1; }
    for (int i = 0; i < g->enemies.count; i++) {
        Enemy *e = &g->enemies.enemies[i];
        if (fread(&e->x, sizeof(e->x), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&e->y, sizeof(e->y), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&e->angle, sizeof(e->angle), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&e->health, sizeof(e->health), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&e->active, sizeof(e->active), 1, f) != 1) { fclose(f); return -1; }
        int estate;
        if (fread(&estate, sizeof(estate), 1, f) != 1) { fclose(f); return -1; }
        e->state = (EnemyState)estate;
        if (fread(&e->attack_timer, sizeof(e->attack_timer), 1, f) != 1) { fclose(f); return -1; }
        if (ver >= 6) {
            if (fread(&e->attack_flash_timer, sizeof(e->attack_flash_timer), 1, f) != 1) { fclose(f); return -1; }
        } else {
            e->attack_flash_timer = 0.0f;
        }
        int etype;
        if (fread(&etype, sizeof(etype), 1, f) != 1) { fclose(f); return -1; }
        e->type = (EnemyType)etype;
        if (fread(&e->walk_frame, sizeof(e->walk_frame), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&e->walk_timer, sizeof(e->walk_timer), 1, f) != 1) { fclose(f); return -1; }
        if (ver >= 9) {
            if (fread(&e->reinforcements_called, sizeof(e->reinforcements_called), 1, f) != 1) { fclose(f); return -1; }
        } else {
            e->reinforcements_called = 0;
        }
    }

    if (fread(&g->items.count, sizeof(g->items.count), 1, f) != 1) { fclose(f); return -1; }
    if (g->items.count < 0 || g->items.count > MAX_ITEMS) { fclose(f); return -1; }
    for (int i = 0; i < g->items.count; i++) {
        Item *it = &g->items.items[i];
        if (fread(&it->x, sizeof(it->x), 1, f) != 1) { fclose(f); return -1; }
        if (fread(&it->y, sizeof(it->y), 1, f) != 1) { fclose(f); return -1; }
        int itype;
        if (fread(&itype, sizeof(itype), 1, f) != 1) { fclose(f); return -1; }
        it->type = (ItemType)itype;
        if (fread(&it->active, sizeof(it->active), 1, f) != 1) { fclose(f); return -1; }
    }

    if (ver >= 7) {
        if (fread(&loaded_settings.music_on, sizeof(loaded_settings.music_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (fread(&loaded_settings.sound_on, sizeof(loaded_settings.sound_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (fread(&loaded_settings.minimap_on, sizeof(loaded_settings.minimap_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (fread(&loaded_settings.enemy_positions_on, sizeof(loaded_settings.enemy_positions_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (ver >= 13 && fread(&loaded_settings.weapon_pickups_on, sizeof(loaded_settings.weapon_pickups_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (ver >= 14 && fread(&loaded_settings.enemy_health_bars_on, sizeof(loaded_settings.enemy_health_bars_on), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        loaded_settings.music_on = loaded_settings.music_on != 0;
        loaded_settings.sound_on = loaded_settings.sound_on != 0;
        loaded_settings.minimap_on = loaded_settings.minimap_on != 0;
        loaded_settings.enemy_positions_on = loaded_settings.enemy_positions_on != 0;
        loaded_settings.weapon_pickups_on = loaded_settings.weapon_pickups_on != 0;
        loaded_settings.enemy_health_bars_on = loaded_settings.enemy_health_bars_on != 0;
    }

    int mw, mh;
    if (fread(&mw, sizeof(mw), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&mh, sizeof(mh), 1, f) != 1) { fclose(f); return -1; }
    if (mw == m->width && mh == m->height) {
        fread(m->cells, sizeof(int) * mw * mh, 1, f);
    }

    fclose(f);
    *s = loaded_settings;
    return 0;
}
