#include "ui/landing.h"
#include "ui/font.h"

/* --- landing screen --- */

static const char *LANDING_LABELS[] = { "NEW GAME", "LOAD", "QUIT" };
static const int   LANDING_ENABLED[] = { 1, 0, 1 };
#define LANDING_COUNT 3

static int landing_selected = 0;

static int landing_next(int cur, int dir) {
    int n = (cur + dir + LANDING_COUNT) % LANDING_COUNT;
    while (!LANDING_ENABLED[n]) {
        n = (n + dir + LANDING_COUNT) % LANDING_COUNT;
    }
    return n;
}

void landing_reset(void) {
    landing_selected = 0;
}

static SDL_Rect item_rect(const char *label, int cx, int y) {
    int pw = font_str_px_w(label) + 24;
    return (SDL_Rect){ cx - pw / 2, y - 4, pw, FONT_CH + 8 };
}

static void draw_item(SDL_Renderer *r, const char *label, int cx, int y, int enabled, int selected) {
    SDL_Rect bg = item_rect(label, cx, y);
    if (selected) {
        SDL_SetRenderDrawColor(r, 140, 100, 20, 255);
    } else {
        SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    }
    SDL_RenderFillRect(r, &bg);
    SDL_Color col;
    if (!enabled) {
        col = (SDL_Color){ 70, 70, 70, 255 };
    } else if (selected) {
        col = (SDL_Color){ 255, 230, 80, 255 };
    } else {
        col = (SDL_Color){ 220, 220, 220, 255 };
    }
    font_draw_string(r, label, cx - font_str_px_w(label) / 2, y, col);
}

void landing_render(SDL_Renderer *r, int sw, int sh) {
    SDL_SetRenderDrawColor(r, 70, 0, 0, 255);
    SDL_Rect bg = { 0, 0, sw, sh };
    SDL_RenderFillRect(r, &bg);

    SDL_Color gold = { 220, 180, 50, 255 };
    const char *title = "WOLFENSTEIN 2026";
    font_draw_string(r, title, (sw - font_str_px_w(title)) / 2, sh / 4, gold);

    int cx = sw / 2;
    int by = sh / 2;
    int gap = FONT_LHEIGHT + 10;

    for (int i = 0; i < LANDING_COUNT; i++) {
        draw_item(r, LANDING_LABELS[i], cx, by + i * gap, LANDING_ENABLED[i], landing_selected == i);
    }
}

LandingResult landing_handle_event(const SDL_Event *e, int sw, int sh) {
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_DOWN) {
            landing_selected = landing_next(landing_selected, 1);
            return LANDING_NONE;
        }
        if (k == SDLK_UP) {
            landing_selected = landing_next(landing_selected, -1);
            return LANDING_NONE;
        }
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            if (landing_selected == 0) { return LANDING_NEW_GAME; }
            if (landing_selected == 2) { return LANDING_QUIT; }
        }
    }
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;
        int cx = sw / 2;
        int by = sh / 2;
        int gap = FONT_LHEIGHT + 10;
        for (int i = 0; i < LANDING_COUNT; i++) {
            if (!LANDING_ENABLED[i]) { continue; }
            SDL_Rect rect = item_rect(LANDING_LABELS[i], cx, by + i * gap);
            if (mx >= rect.x && mx < rect.x + rect.w && my >= rect.y && my < rect.y + rect.h) {
                landing_selected = i;
                if (i == 0) { return LANDING_NEW_GAME; }
                if (i == 2) { return LANDING_QUIT; }
            }
        }
    }
    return LANDING_NONE;
}

/* --- difficulty screen --- */

static const char *DIFF_NAMES[DIFF_COUNT] = {
    "CAN I PLAY, DADDY?",
    "DONT HURT ME.",
    "BRING EM ON!",
    "I AM DEATH INCARNATE!"
};
static const int DIFF_ENABLED[DIFF_COUNT] = { 1, 0, 0, 0 };

static int diff_selected = 0;

static int diff_next(int cur, int dir) {
    int n = (cur + dir + DIFF_COUNT) % DIFF_COUNT;
    while (!DIFF_ENABLED[n]) {
        n = (n + dir + DIFF_COUNT) % DIFF_COUNT;
    }
    return n;
}

void difficulty_screen_reset(void) {
    diff_selected = 0;
}

void difficulty_screen_render(SDL_Renderer *r, int sw, int sh) {
    SDL_SetRenderDrawColor(r, 70, 0, 0, 255);
    SDL_Rect bg = { 0, 0, sw, sh };
    SDL_RenderFillRect(r, &bg);

    SDL_Color gold = { 220, 180, 50, 255 };
    const char *title = "HOW TOUGH ARE YOU?";
    font_draw_string(r, title, (sw - font_str_px_w(title)) / 2, sh / 4, gold);

    int cx = sw / 2;
    int by = sh / 2 - (DIFF_COUNT * (FONT_LHEIGHT + 10)) / 2;

    for (int i = 0; i < DIFF_COUNT; i++) {
        draw_item(r, DIFF_NAMES[i], cx, by + i * (FONT_LHEIGHT + 10), DIFF_ENABLED[i], diff_selected == i);
    }
}

Difficulty difficulty_screen_handle_event(const SDL_Event *e, int sw, int sh) {
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_DOWN) {
            diff_selected = diff_next(diff_selected, 1);
            return DIFF_COUNT;
        }
        if (k == SDLK_UP) {
            diff_selected = diff_next(diff_selected, -1);
            return DIFF_COUNT;
        }
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            if (DIFF_ENABLED[diff_selected]) {
                return (Difficulty)diff_selected;
            }
        }
    }
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;
        int cx = sw / 2;
        int by = sh / 2 - (DIFF_COUNT * (FONT_LHEIGHT + 10)) / 2;
        for (int i = 0; i < DIFF_COUNT; i++) {
            if (!DIFF_ENABLED[i]) { continue; }
            SDL_Rect rect = item_rect(DIFF_NAMES[i], cx, by + i * (FONT_LHEIGHT + 10));
            if (mx >= rect.x && mx < rect.x + rect.w && my >= rect.y && my < rect.y + rect.h) {
                return (Difficulty)i;
            }
        }
    }
    return DIFF_COUNT;
}
