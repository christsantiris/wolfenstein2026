#include "player.h"

#define DEFAULT_MOVE_SPEED 3.0f
#define DEFAULT_ROT_SPEED  2.0f

void player_init(Player *p, float x, float y, float angle) {
    p->x = x;
    p->y = y;
    p->angle = angle;
    p->move_speed = DEFAULT_MOVE_SPEED;
    p->rot_speed = DEFAULT_ROT_SPEED;
}
