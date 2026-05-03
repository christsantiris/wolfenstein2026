#include "ui/menu.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define SCALE     2
#define CW        (5 * SCALE)
#define CH        (7 * SCALE)
#define CSTRIDE   (CW + SCALE)
#define LHEIGHT   (CH + SCALE * 4)

#define MENU_ITEM_COUNT    4
#define MENU_ITEM_MUSIC    0
#define MENU_ITEM_SOUND    1
#define MENU_ITEM_NEW_GAME 2
#define MENU_ITEM_QUIT     3

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

static int button_hit(int mx, int my, int bx, int by, const char *label) {
    int pw = str_px_w(label) + 16;
    return mx >= bx && mx < bx + pw && my >= by - 4 && my < by - 4 + CH + 8;
}

typedef struct {
    int bx, by, bw, bh;
    int music_y, sound_y;
    int save_x, load_x, row1_y;
    int new_game_x, quit_x, row2_y;
} MenuLayout;

static void menu_compute_layout(int sw, int sh, MenuLayout *l) {
    l->bw = 440;
    l->bh = 474 + LHEIGHT;
    l->bx = (sw - l->bw) / 2;
    l->by = (sh - l->bh) / 2;

    int y = l->by + 18 + LHEIGHT + 4 + 10;
    y += LHEIGHT;
    y += LHEIGHT * 8 + 4;
    y += 10;
    y += LHEIGHT;
    y += LHEIGHT;
    l->music_y = y; y += LHEIGHT;
    l->sound_y = y; y += LHEIGHT + 4 + 14;

    l->save_x = l->bx + l->bw / 2 - str_px_w("SAVE") - 24;
    l->load_x = l->bx + l->bw / 2 + 8;
    l->row1_y = y;
    y += LHEIGHT + 8;

    int ng_pw = str_px_w("NEW GAME") + 16;
    int qt_pw = str_px_w("QUIT") + 16;
    int pair_w = ng_pw + 16 + qt_pw;
    l->new_game_x = l->bx + (l->bw - pair_w) / 2;
    l->quit_x = l->new_game_x + ng_pw + 16;
    l->row2_y = y;
}

static MenuAction activate_item(Menu *m, int item) {
    if (item == MENU_ITEM_MUSIC) {
        m->music_on = !m->music_on;
        return MENU_ACTION_MUSIC_TOGGLE;
    }
    if (item == MENU_ITEM_SOUND) {
        m->sound_on = !m->sound_on;
        return MENU_ACTION_SOUND_TOGGLE;
    }
    if (item == MENU_ITEM_NEW_GAME) {
        return MENU_ACTION_NEW_GAME;
    }
    if (item == MENU_ITEM_QUIT) {
        return MENU_ACTION_QUIT;
    }
    return MENU_ACTION_NONE;
}

MenuAction menu_handle_event(Menu *m, const SDL_Event *e, int sw, int sh) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_UP || e->key.keysym.sym == SDLK_KP_8) {
            m->selected = (m->selected - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
            return MENU_ACTION_NONE;
        }
        if (e->key.keysym.sym == SDLK_DOWN || e->key.keysym.sym == SDLK_KP_2) {
            m->selected = (m->selected + 1) % MENU_ITEM_COUNT;
            return MENU_ACTION_NONE;
        }
        if (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_KP_ENTER || e->key.keysym.sym == SDLK_SPACE) {
            return activate_item(m, m->selected);
        }
    }

    if (e->type != SDL_MOUSEBUTTONDOWN || e->button.button != SDL_BUTTON_LEFT) {
        return MENU_ACTION_NONE;
    }
    int mx = e->button.x;
    int my = e->button.y;

    MenuLayout l;
    menu_compute_layout(sw, sh, &l);

    if (my >= l.music_y && my < l.music_y + CH &&
        mx >= l.bx && mx < l.bx + l.bw) {
        m->selected = MENU_ITEM_MUSIC;
        return activate_item(m, MENU_ITEM_MUSIC);
    }
    if (my >= l.sound_y && my < l.sound_y + CH &&
        mx >= l.bx && mx < l.bx + l.bw) {
        m->selected = MENU_ITEM_SOUND;
        return activate_item(m, MENU_ITEM_SOUND);
    }
    if (button_hit(mx, my, l.new_game_x, l.row2_y, "NEW GAME")) {
        m->selected = MENU_ITEM_NEW_GAME;
        return activate_item(m, MENU_ITEM_NEW_GAME);
    }
    if (button_hit(mx, my, l.quit_x, l.row2_y, "QUIT")) {
        m->selected = MENU_ITEM_QUIT;
        return activate_item(m, MENU_ITEM_QUIT);
    }
    return MENU_ACTION_NONE;
}

void menu_render(SDL_Renderer *r, const Menu *m, int screen_w, int screen_h) {
    MenuLayout l;
    menu_compute_layout(screen_w, screen_h, &l);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, screen_w, screen_h };
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
    SDL_Rect box = { l.bx, l.by, l.bw, l.bh };
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, 180, 140, 40, 255);
    SDL_RenderDrawRect(r, &box);

    SDL_Color gold = { 220, 180, 50,  255 };
    SDL_Color dim  = { 100, 100, 100, 255 };

    int y = l.by + 18;
    draw_centered(r, "OPTIONS", l.bx, y, l.bw, gold);
    y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, l.bx + 10, y, l.bx + l.bw - 10, y);
    y += 10;

    draw_centered(r, "CONTROLS", l.bx, y, l.bw, dim);
    y += LHEIGHT;
    draw_row(r, "FORWARD",      "W",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "BACK",         "S",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "STRAFE LEFT",  "A",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "STRAFE RIGHT", "D",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "SHOOT",        "SPACE", l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "RELOAD",       "R",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "OPEN DOOR",    "O",     l.bx, y, l.bw); y += LHEIGHT;
    draw_row(r, "FULLSCREEN",   "F",     l.bx, y, l.bw); y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, l.bx + 10, y, l.bx + l.bw - 10, y);
    y += 10;

    static const char *DIFF_LABELS[4] = {
        "CAN I PLAY, DADDY?",
        "DONT HURT ME.",
        "BRING EM ON",
        "I AM DEATH INCARNATE"
    };
    int di = (m->difficulty >= 0 && m->difficulty < 4) ? m->difficulty : 0;
    draw_centered(r, "SETTINGS", l.bx, y, l.bw, dim);
    y += LHEIGHT;
    draw_row(r, "DIFFICULTY",  DIFF_LABELS[di],             l.bx, y, l.bw); y += LHEIGHT;
    if (m->selected == MENU_ITEM_MUSIC) {
        SDL_SetRenderDrawColor(r, 70, 52, 12, 255);
        SDL_Rect hl = { l.bx + 4, y - 2, l.bw - 8, CH + 4 };
        SDL_RenderFillRect(r, &hl);
    }
    draw_row(r, "MUSIC",       m->music_on ? "ON" : "OFF",  l.bx, y, l.bw);
    y += LHEIGHT;
    if (m->selected == MENU_ITEM_SOUND) {
        SDL_SetRenderDrawColor(r, 70, 52, 12, 255);
        SDL_Rect hl = { l.bx + 4, y - 2, l.bw - 8, CH + 4 };
        SDL_RenderFillRect(r, &hl);
    }
    draw_row(r, "SOUND",       m->sound_on ? "ON" : "OFF", l.bx, y, l.bw);
    y += LHEIGHT + 4;

    SDL_SetRenderDrawColor(r, 80, 60, 20, 255);
    SDL_RenderDrawLine(r, l.bx + 10, y, l.bx + l.bw - 10, y);
    y += 14;

    draw_button(r, "SAVE", l.save_x, y);
    draw_button(r, "LOAD", l.load_x, y);
    y += LHEIGHT + 8;

    draw_button(r, "NEW GAME", l.new_game_x, y);
    draw_button(r, "QUIT",     l.quit_x,     y);
    if (m->selected == MENU_ITEM_NEW_GAME) {
        int pw = str_px_w("NEW GAME") + 16;
        SDL_SetRenderDrawColor(r, 220, 180, 50, 255);
        SDL_Rect border = { l.new_game_x - 1, y - 5, pw + 2, CH + 10 };
        SDL_RenderDrawRect(r, &border);
    }
    if (m->selected == MENU_ITEM_QUIT) {
        int pw = str_px_w("QUIT") + 16;
        SDL_SetRenderDrawColor(r, 220, 180, 50, 255);
        SDL_Rect border = { l.quit_x - 1, y - 5, pw + 2, CH + 10 };
        SDL_RenderDrawRect(r, &border);
    }
}

static void game_over_button_rects(int sw, int sh, int score_count, SDL_Rect *rn, SDL_Rect *rl, SDL_Rect *rq) {
    int nw = str_px_w("NEW GAME") + 16;
    int lw = str_px_w("LOAD") + 16;
    int qw = str_px_w("QUIT") + 16;
    int bh = CH + 8;
    int total = nw + 20 + lw + 20 + qw;
    int bx = sw / 2 - total / 2;
    /* match the render layout: start + GAME OVER + SCORE + RANK + title + entries + gap */
    int by = sh / 2 - 100
           + (LHEIGHT + 4)
           + (LHEIGHT + 2)
           + (LHEIGHT + 12)
           + (LHEIGHT + 4)
           + score_count * LHEIGHT
           + 12;
    rn->x = bx;                        rn->y = by; rn->w = nw; rn->h = bh;
    rl->x = bx + nw + 20;             rl->y = by; rl->w = lw; rl->h = bh;
    rq->x = bx + nw + 20 + lw + 20;  rq->y = by; rq->w = qw; rq->h = bh;
}

void game_over_render(SDL_Renderer *r, int sw, int sh, int score, int rank, const HighScoreTable *t) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 210);
    SDL_Rect overlay = { 0, 0, sw, sh };
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color red   = { 200,  40,  40, 255 };
    SDL_Color gold  = { 220, 180,  50, 255 };
    SDL_Color white = { 220, 220, 220, 255 };
    SDL_Color grey  = { 140, 140, 140, 255 };

    int cx = sw / 2;
    int y = sh / 2 - 100;

    draw_string(r, "GAME OVER", cx - str_px_w("GAME OVER") / 2, y, red);
    y += LHEIGHT + 4;

    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "SCORE  %d", score);
    draw_string(r, score_buf, cx - str_px_w(score_buf) / 2, y, gold);
    y += LHEIGHT + 2;

    if (rank > 0) {
        char rank_buf[32];
        snprintf(rank_buf, sizeof(rank_buf), "RANK  #%d", rank);
        draw_string(r, rank_buf, cx - str_px_w(rank_buf) / 2, y, gold);
    }
    y += LHEIGHT + 12;

    draw_string(r, "HIGH SCORES", cx - str_px_w("HIGH SCORES") / 2, y, white);
    y += LHEIGHT + 4;

    for (int i = 0; i < t->count; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%d.  %d", i + 1, t->scores[i]);
        SDL_Color col = (i + 1 == rank) ? gold : grey;
        draw_string(r, line, cx - str_px_w(line) / 2, y, col);
        y += LHEIGHT;
    }

    SDL_Rect rn, rl, rq;
    game_over_button_rects(sw, sh, t->count, &rn, &rl, &rq);
    draw_button(r, "NEW GAME", rn.x, rn.y + 4);
    draw_button(r, "LOAD",     rl.x, rl.y + 4);
    draw_button(r, "QUIT",     rq.x, rq.y + 4);
}

GameOverResult game_over_handle_event(const SDL_Event *e, int sw, int sh, int score_count) {
    if (e->type != SDL_MOUSEBUTTONDOWN || e->button.button != SDL_BUTTON_LEFT) {
        return GAME_OVER_NONE;
    }
    int mx = e->button.x;
    int my = e->button.y;
    SDL_Rect rn, rl, rq;
    game_over_button_rects(sw, sh, score_count, &rn, &rl, &rq);
    if (mx >= rn.x && mx < rn.x + rn.w && my >= rn.y && my < rn.y + rn.h) {
        return GAME_OVER_NEW_GAME;
    }
    if (mx >= rq.x && mx < rq.x + rq.w && my >= rq.y && my < rq.y + rq.h) {
        return GAME_OVER_QUIT;
    }
    return GAME_OVER_NONE;
}
