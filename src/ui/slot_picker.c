#include "ui/slot_picker.h"
#include "ui/font.h"
#include "systems/save_load.h"
#include <stdio.h>

#define BOX_W 300
#define BOX_H 220
#define SLOT_H (FONT_CH + 16)

static void slot_rect(int bx, int by, int slot, SDL_Rect *out) {
    int inner_y = by + FONT_LHEIGHT + 20 + slot * (SLOT_H + 8);
    out->x = bx + 16;
    out->y = inner_y;
    out->w = BOX_W - 32;
    out->h = SLOT_H;
}

void slot_picker_open(SlotPicker *sp, int is_save) {
    sp->is_open = 1;
    sp->is_save = is_save;
}

SlotResult slot_picker_handle_event(SlotPicker *sp, const SDL_Event *e, int sw, int sh, int *out_slot) {
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE) {
        sp->is_open = 0;
        return SLOT_RESULT_CANCEL;
    }

    if (e->type != SDL_MOUSEBUTTONDOWN || e->button.button != SDL_BUTTON_LEFT) {
        return SLOT_RESULT_NONE;
    }

    int bx = (sw - BOX_W) / 2;
    int by = (sh - BOX_H) / 2;
    int mx = e->button.x;
    int my = e->button.y;

    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        int exists = save_slot_exists(i);
        if (!sp->is_save && !exists) {
            continue;
        }
        SDL_Rect r;
        slot_rect(bx, by, i, &r);
        if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
            sp->is_open = 0;
            *out_slot = i;
            return SLOT_RESULT_SELECTED;
        }
    }

    return SLOT_RESULT_NONE;
}

void slot_picker_render(SDL_Renderer *r, const SlotPicker *sp, int sw, int sh) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
    SDL_Rect overlay = { 0, 0, sw, sh };
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    int bx = (sw - BOX_W) / 2;
    int by = (sh - BOX_H) / 2;

    SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
    SDL_Rect box = { bx, by, BOX_W, BOX_H };
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, 180, 140, 40, 255);
    SDL_RenderDrawRect(r, &box);

    SDL_Color gold = { 220, 180, 50,  255 };
    SDL_Color grey = { 100, 100, 100, 255 };
    SDL_Color white = { 220, 220, 220, 255 };
    SDL_Color green = { 80, 180, 80, 255 };

    const char *title = sp->is_save ? "SAVE GAME" : "LOAD GAME";
    int tx = bx + (BOX_W - font_str_px_w(title)) / 2;
    font_draw_string(r, title, tx, by + 14, gold);

    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        int exists = save_slot_exists(i);
        int clickable = sp->is_save || exists;

        SDL_Rect sr;
        slot_rect(bx, by, i, &sr);

        SDL_Color bg_col = clickable
            ? (SDL_Color){ 55, 45, 15, 255 }
            : (SDL_Color){ 35, 35, 35, 255 };
        SDL_SetRenderDrawColor(r, bg_col.r, bg_col.g, bg_col.b, 255);
        SDL_RenderFillRect(r, &sr);
        SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
        SDL_RenderDrawRect(r, &sr);

        char label[16];
        snprintf(label, sizeof(label), "SLOT %d", i + 1);
        int lx = sr.x + 10;
        int ly = sr.y + (SLOT_H - FONT_CH) / 2;
        font_draw_string(r, label, lx, ly, clickable ? white : grey);

        const char *status = exists ? "SAVED" : "EMPTY";
        SDL_Color sc = exists ? green : grey;
        int sx = sr.x + sr.w - font_str_px_w(status) - 10;
        font_draw_string(r, status, sx, ly, sc);
    }

    const char *hint = "ESC TO CANCEL";
    int hx = bx + (BOX_W - font_str_px_w(hint)) / 2;
    font_draw_string(r, hint, hx, by + BOX_H - FONT_LHEIGHT - 6, grey);
}
