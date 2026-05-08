#include "systems/save_load.h"
#include "core/player.h"
#include "core/game.h"
#include "core/map.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#define SAVE_MAGIC   "WOLF2026"
#define SAVE_VERSION 2

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

int save_game(int slot, int level, const Player *p, const GameState *g, const Map *m) {
    mkdir("saves", 0755);
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
        int etype = (int)e->type;
        fwrite(&etype, sizeof(etype), 1, f);
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

    fwrite(&m->width, sizeof(m->width), 1, f);
    fwrite(&m->height, sizeof(m->height), 1, f);
    fwrite(m->cells, sizeof(int) * m->width * m->height, 1, f);

    fclose(f);
    return 0;
}

int load_game(int slot, int *level, Player *p, GameState *g, Map *m) {
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
    if (fread(&ver, sizeof(ver), 1, f) != 1 || ver != SAVE_VERSION) {
        fclose(f);
        return -1;
    }

    if (fread(level, sizeof(*level), 1, f) != 1) {
        fclose(f);
        return -1;
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
    if (fread(g->reserve_ammo_per_gun, sizeof(g->reserve_ammo_per_gun), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&g->score, sizeof(g->score), 1, f) != 1) { fclose(f); return -1; }
    int wtype;
    if (fread(&wtype, sizeof(wtype), 1, f) != 1) { fclose(f); return -1; }
    g->current_weapon = *weapon_def((GunType)wtype);
    if (fread(g->has_weapon, sizeof(g->has_weapon), 1, f) != 1) { fclose(f); return -1; }
    if (fread(g->ammo_per_gun, sizeof(g->ammo_per_gun), 1, f) != 1) { fclose(f); return -1; }

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
        int etype;
        if (fread(&etype, sizeof(etype), 1, f) != 1) { fclose(f); return -1; }
        e->type = (EnemyType)etype;
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

    int mw, mh;
    if (fread(&mw, sizeof(mw), 1, f) != 1) { fclose(f); return -1; }
    if (fread(&mh, sizeof(mh), 1, f) != 1) { fclose(f); return -1; }
    if (mw == m->width && mh == m->height) {
        fread(m->cells, sizeof(int) * mw * mh, 1, f);
    }

    fclose(f);
    return 0;
}
