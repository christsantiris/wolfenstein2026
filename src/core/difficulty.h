#ifndef DIFFICULTY_H
#define DIFFICULTY_H

typedef enum {
    DIFF_CAN_I_PLAY_DADDY = 0,
    DIFF_DONT_HURT_ME,
    DIFF_BRING_EM_ON,
    DIFF_I_AM_DEATH_INCARNATE,
    DIFF_COUNT
} Difficulty;

typedef struct {
    const char *name;
    int enabled;
    int enemy_damage;
    float enemy_speed;
    float attack_cooldown;
    float sight_range;
} DifficultyDef;

const DifficultyDef *difficulty_get(Difficulty d);

#endif
