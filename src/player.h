#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    float x;
    float y;
    float angle;
    float move_speed;
    float rot_speed;
} Player;

void player_init(Player *p, float x, float y, float angle);

#endif
