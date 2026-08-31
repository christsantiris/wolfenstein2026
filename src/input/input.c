#include "input/input.h"
#include <SDL2/SDL.h>
#include <math.h>

#define TURN_START_SPEED 0.70f
#define TURN_ACCELERATION 3.75f
#define AIM_TURN_SPEED 0.85f
#define AIM_CENTER_MAX_DISTANCE 8.0f
#define AIM_VISIBILITY_STEP 0.1f

static float turn_speed = 0.0f;
static int turn_direction = 0;

static float normalize_angle(float angle) {
    while (angle > M_PI) {
        angle -= 2.0f * (float)M_PI;
    }
    while (angle < -M_PI) {
        angle += 2.0f * (float)M_PI;
    }
    return angle;
}

static int aim_target_visible(const Player *p, const Enemy *e, const Map *m) {
    float dx = e->x - p->x;
    float dy = e->y - p->y;
    float distance = sqrtf(dx * dx + dy * dy);
    int steps = (int)(distance / AIM_VISIBILITY_STEP) + 1;
    for (int step = 1; step < steps; step++) {
        float x = p->x + dx * step / steps;
        float y = p->y + dy * step / steps;
        if (map_is_wall(m, (int)x, (int)y)) {
            return 0;
        }
    }
    return 1;
}

static float center_crossed_target(const Player *p, const Map *m, const EnemyList *enemies, float proposed_angle, int direction) {
    float best_travel = (float)M_PI;
    float centered_angle = proposed_angle;
    for (int i = 0; i < enemies->count; i++) {
        const Enemy *enemy = &enemies->enemies[i];
        if (!enemy->active) {
            continue;
        }
        float dx = enemy->x - p->x;
        float dy = enemy->y - p->y;
        float distance = sqrtf(dx * dx + dy * dy);
        if (distance > AIM_CENTER_MAX_DISTANCE || !aim_target_visible(p, enemy, m)) {
            continue;
        }
        float enemy_angle = atan2f(dy, dx);
        float old_diff = normalize_angle(enemy_angle - p->angle);
        float new_diff = normalize_angle(enemy_angle - proposed_angle);
        int crossed = direction < 0 ? old_diff < 0.0f && new_diff > 0.0f : old_diff > 0.0f && new_diff < 0.0f;
        float travel = fabsf(old_diff);
        if (crossed && travel < best_travel) {
            best_travel = travel;
            centered_angle = enemy_angle;
        }
    }
    return centered_angle;
}

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

void input_update(Player *p, const Map *m, const EnemyList *enemies, float dt) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    float move = p->move_speed * dt;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        try_move(p, m, cosf(p->angle) * move, sinf(p->angle) * move);
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        try_move(p, m, -cosf(p->angle) * move, -sinf(p->angle) * move);
    }

    int aim_turn = 0;
    if (keys[SDL_SCANCODE_LEFT] && !keys[SDL_SCANCODE_RIGHT]) {
        aim_turn = -1;
    } else if (keys[SDL_SCANCODE_RIGHT] && !keys[SDL_SCANCODE_LEFT]) {
        aim_turn = 1;
    }

    int requested_turn = 0;
    if (keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D]) {
        requested_turn = -1;
    } else if (keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A]) {
        requested_turn = 1;
    }

    if (aim_turn != 0) {
        turn_speed = 0.0f;
        float proposed_angle = p->angle + aim_turn * AIM_TURN_SPEED * dt;
        p->angle = center_crossed_target(p, m, enemies, proposed_angle, aim_turn);
    } else if (requested_turn == 0) {
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
