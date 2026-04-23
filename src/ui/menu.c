#include "ui/menu.h"
#include <stdint.h>
#include <string.h>

#define SCALE     2
#define CW        (5 * SCALE)
#define CH        (7 * SCALE)
#define CSTRIDE   (CW + SCALE)
#define LHEIGHT   (CH + SCALE * 4)

static const uint8_t GLYPHS[96][7] = {
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* ' '  32 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* !    33 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* "    34 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* #    35 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* $    36 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* %    37 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* &    38 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* '    39 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* (    40 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* )    41 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* *    42 */
    { 0x00,0x00,0x04,0x1F,0x04,0x00,0x00 }, /* +    43 */
    { 0x00,0x00,0x00,0x00,0x00,0x04,0x08 }, /* ,    44 */
    { 0x00,0x00,0x00,0x1F,0x00,0x00,0x00 }, /* -    45 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x04 }, /* .    46 */
    { 0x00,0x01,0x02,0x04,0x08,0x10,0x00 }, /* /    47 */
    { 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E }, /* 0    48 */
    { 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E }, /* 1    49 */
    { 0x0E,0x11,0x01,0x02,0x04,0x08,0x1F }, /* 2    50 */
    { 0x0E,0x11,0x01,0x06,0x01,0x11,0x0E }, /* 3    51 */
    { 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 }, /* 4    52 */
    { 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E }, /* 5    53 */
    { 0x06,0x08,0x10,0x1E,0x11,0x11,0x0E }, /* 6    54 */
    { 0x1F,0x01,0x02,0x04,0x08,0x08,0x08 }, /* 7    55 */
    { 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E }, /* 8    56 */
    { 0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E }, /* 9    57 */
    { 0x00,0x04,0x00,0x00,0x04,0x00,0x00 }, /* :    58 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* ;    59 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* <    60 */
    { 0x00,0x00,0x1F,0x00,0x1F,0x00,0x00 }, /* =    61 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* >    62 */
    { 0x0E,0x11,0x01,0x06,0x04,0x00,0x04 }, /* ?    63 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* @    64 */
    { 0x04,0x0A,0x11,0x1F,0x11,0x11,0x11 }, /* A    65 */
    { 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E }, /* B    66 */
    { 0x0E,0x11,0x10,0x10,0x10,0x11,0x0E }, /* C    67 */
    { 0x1C,0x12,0x11,0x11,0x11,0x12,0x1C }, /* D    68 */
    { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F }, /* E    69 */
    { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 }, /* F    70 */
    { 0x0E,0x11,0x10,0x17,0x11,0x11,0x0F }, /* G    71 */
    { 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 }, /* H    72 */
    { 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E }, /* I    73 */
    { 0x07,0x02,0x02,0x02,0x02,0x12,0x0C }, /* J    74 */
    { 0x11,0x12,0x14,0x18,0x14,0x12,0x11 }, /* K    75 */
    { 0x10,0x10,0x10,0x10,0x10,0x10,0x1F }, /* L    76 */
    { 0x11,0x1B,0x15,0x11,0x11,0x11,0x11 }, /* M    77 */
    { 0x11,0x19,0x15,0x13,0x11,0x11,0x11 }, /* N    78 */
    { 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E }, /* O    79 */
    { 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 }, /* P    80 */
    { 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D }, /* Q    81 */
    { 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 }, /* R    82 */
    { 0x0E,0x10,0x10,0x0E,0x01,0x01,0x0E }, /* S    83 */
    { 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 }, /* T    84 */
    { 0x11,0x11,0x11,0x11,0x11,0x11,0x0E }, /* U    85 */
    { 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 }, /* V    86 */
    { 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 }, /* W    87 */
    { 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 }, /* X    88 */
    { 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 }, /* Y    89 */
    { 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F }, /* Z    90 */
    { 0x0E,0x08,0x08,0x08,0x08,0x08,0x0E }, /* [    91 */
    { 0x00,0x10,0x08,0x04,0x02,0x01,0x00 }, /* \    92 */
    { 0x0E,0x02,0x02,0x02,0x02,0x02,0x0E }, /* ]    93 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* ^    94 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x1F }, /* _    95 */
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* `   127 */
};

static void draw_char(SDL_Renderer *r, char c, int x, int y, SDL_Color col) {
    if (c < 32 || c > 127) {
        c = ' ';
    }
    const uint8_t *g = GLYPHS[c - 32];
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, 255);
    for (int row = 0; row < 7; row++) {
        for (int bit = 0; bit < 5; bit++) {
            if (g[row] & (1 << (4 - bit))) {
                SDL_Rect px = { x + bit * SCALE, y + row * SCALE, SCALE, SCALE };
                SDL_RenderFillRect(r, &px);
            }
        }
    }
}

static void draw_string(SDL_Renderer *r, const char *s, int x, int y, SDL_Color col) {
    int cx = x;
    while (*s) {
        draw_char(r, *s, cx, y, col);
        cx += CSTRIDE;
        s++;
    }
}

static int str_px_w(const char *s) {
    return (int)strlen(s) * CSTRIDE;
}

static void draw_row(SDL_Renderer *r, const char *label, const char *value, int bx, int y, int inner_w) {
    SDL_Color white = { 220, 220, 220, 255 };
    SDL_Color grey  = { 140, 140, 140, 255 };
    draw_string(r, label, bx + 20, y, white);
    draw_string(r, value, bx + inner_w - str_px_w(value) - 20, y, grey);
}

static void draw_centered(SDL_Renderer *r, const char *s, int bx, int y, int inner_w, SDL_Color col) {
    int tx = bx + (inner_w - str_px_w(s)) / 2;
    draw_string(r, s, tx, y, col);
}

static void draw_button(SDL_Renderer *r, const char *label, int x, int y) {
    int pw = str_px_w(label) + 16;
    SDL_SetRenderDrawColor(r, 80, 80, 80, 255);
    SDL_Rect bg = { x, y - 4, pw, CH + 8 };
    SDL_RenderFillRect(r, &bg);
    SDL_Color col = { 220, 220, 220, 255 };
    draw_string(r, label, x + 8, y, col);
}

void menu_handle_event(Menu *m, const SDL_Event *e, int *running) {
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;

        int bw = 440;
        int bh = 420;
        int bx = (800 - bw) / 2;
        int by = (600 - bh) / 2;
        int quit_y = by + bh - 50;
        int quit_x = bx + (bw - str_px_w("QUIT") - 16) / 2;
        int quit_pw = str_px_w("QUIT") + 16;

        if (mx >= quit_x && mx <= quit_x + quit_pw &&
            my >= quit_y - 4 && my <= quit_y - 4 + CH + 8) {
            *running = 0;
        }
    }
}

void menu_render(SDL_Renderer *r, const Menu *m, int screen_w, int screen_h) {
    (void)m;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, screen_w, screen_h };
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    int bw = 440;
    int bh = 420;
    int bx = (screen_w - bw) / 2;
    int by = (screen_h - bh) / 2;

    SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
    SDL_Rect box = { bx, by, bw, bh };
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, 180, 140, 40, 255);
    SDL_RenderDrawRect(r, &box);

    SDL_Color gold   = { 220, 180, 50,  255 };
    SDL_Color dim    = { 100, 100, 100, 255 };

    int y = by + 18;
    draw_centered(r, "OPTIONS", bx, y, bw, gold);
    y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, bx + 10, y, bx + bw - 10, y);
    y += 10;

    draw_centered(r, "CONTROLS", bx, y, bw, dim);
    y += LHEIGHT;
    draw_row(r, "FORWARD",     "W",     bx, y, bw); y += LHEIGHT;
    draw_row(r, "BACK",        "S",     bx, y, bw); y += LHEIGHT;
    draw_row(r, "STRAFE LEFT", "A",     bx, y, bw); y += LHEIGHT;
    draw_row(r, "STRAFE RIGHT","D",     bx, y, bw); y += LHEIGHT;
    draw_row(r, "SHOOT",       "SPACE", bx, y, bw); y += LHEIGHT;
    draw_row(r, "RELOAD",      "R",     bx, y, bw); y += LHEIGHT;
    draw_row(r, "OPEN DOOR",   "O",     bx, y, bw); y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, bx + 10, y, bx + bw - 10, y);
    y += 10;

    draw_centered(r, "SETTINGS", bx, y, bw, dim);
    y += LHEIGHT;
    draw_row(r, "AUTO RELOAD", "OFF", bx, y, bw); y += LHEIGHT;
    draw_row(r, "MUSIC",       "ON",  bx, y, bw); y += LHEIGHT;
    draw_row(r, "SOUND",       "ON",  bx, y, bw); y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, bx + 10, y, bx + bw - 10, y);
    y += 14;

    int save_x = bx + bw / 2 - str_px_w("SAVE") - 24;
    int load_x = bx + bw / 2 + 8;
    draw_button(r, "SAVE", save_x, y);
    draw_button(r, "LOAD", load_x, y);
    y += LHEIGHT + 8;

    int quit_x = bx + (bw - str_px_w("QUIT") - 16) / 2;
    draw_button(r, "QUIT", quit_x, y);
}

static void game_over_button_rects(int sw, int sh, SDL_Rect *rn, SDL_Rect *rl, SDL_Rect *rq) {
    int nw = str_px_w("NEW GAME") + 16;
    int lw = str_px_w("LOAD") + 16;
    int qw = str_px_w("QUIT") + 16;
    int bh = CH + 8;
    int total = nw + 20 + lw + 20 + qw;
    int bx = sw / 2 - total / 2;
    int by = sh / 2 + 30;
    rn->x = bx;              rn->y = by; rn->w = nw; rn->h = bh;
    rl->x = bx + nw + 20;   rl->y = by; rl->w = lw; rl->h = bh;
    rq->x = bx + nw + 20 + lw + 20; rq->y = by; rq->w = qw; rq->h = bh;
}

void game_over_render(SDL_Renderer *r, int sw, int sh) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 210);
    SDL_Rect overlay = { 0, 0, sw, sh };
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color red = { 200, 40, 40, 255 };
    int tx = sw / 2 - str_px_w("GAME OVER") / 2;
    draw_string(r, "GAME OVER", tx, sh / 2 - 40, red);

    SDL_Rect rn, rl, rq;
    game_over_button_rects(sw, sh, &rn, &rl, &rq);
    draw_button(r, "NEW GAME", rn.x, rn.y + 4);
    draw_button(r, "LOAD",     rl.x, rl.y + 4);
    draw_button(r, "QUIT",     rq.x, rq.y + 4);
}

GameOverResult game_over_handle_event(const SDL_Event *e, int sw, int sh) {
    if (e->type != SDL_MOUSEBUTTONDOWN || e->button.button != SDL_BUTTON_LEFT) {
        return GAME_OVER_NONE;
    }
    int mx = e->button.x;
    int my = e->button.y;
    SDL_Rect rn, rl, rq;
    game_over_button_rects(sw, sh, &rn, &rl, &rq);
    if (mx >= rn.x && mx < rn.x + rn.w && my >= rn.y && my < rn.y + rn.h) {
        return GAME_OVER_NEW_GAME;
    }
    if (mx >= rq.x && mx < rq.x + rq.w && my >= rq.y && my < rq.y + rq.h) {
        return GAME_OVER_QUIT;
    }
    return GAME_OVER_NONE;
}
