#include "input.h"
#include <SDL2/SDL.h>
#include <math.h>

static void try_move(Player *p, const Map *m, float dx, float dy) {
    float nx = p->x + dx;
    float ny = p->y + dy;
    if (!map_is_wall(m, (int)nx, (int)p->y)) {
        p->x = nx;
    }
    if (!map_is_wall(m, (int)p->x, (int)ny)) {
        p->y = ny;
    }
}

void input_update(Player *p, const Map *m, float dt) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    float move = p->move_speed * dt;
    float rot = p->rot_speed * dt;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        try_move(p, m, cosf(p->angle) * move, sinf(p->angle) * move);
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        try_move(p, m, -cosf(p->angle) * move, -sinf(p->angle) * move);
    }
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
        p->angle -= rot;
    }
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
        p->angle += rot;
    }
}
