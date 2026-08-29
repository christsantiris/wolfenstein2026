#include "input/input.h"
#include <SDL2/SDL.h>
#include <math.h>

#define TURN_START_SPEED 0.70f
#define TURN_ACCELERATION 3.75f

static float turn_speed = 0.0f;
static int turn_direction = 0;

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

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        try_move(p, m, cosf(p->angle) * move, sinf(p->angle) * move);
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        try_move(p, m, -cosf(p->angle) * move, -sinf(p->angle) * move);
    }

    int requested_turn = 0;
    if ((keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) && !(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])) {
        requested_turn = -1;
    } else if ((keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) && !(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])) {
        requested_turn = 1;
    }

    if (requested_turn == 0) {
        turn_speed = 0.0f;
    } else {
        if (requested_turn != turn_direction) {
            turn_speed = TURN_START_SPEED;
        } else {
            turn_speed += TURN_ACCELERATION * dt;
        }
        if (turn_speed > p->rot_speed) {
            turn_speed = p->rot_speed;
        }
        p->angle += requested_turn * turn_speed * dt;
    }
    turn_direction = requested_turn;
}
