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
    Difficulty id;
    const char *name;
    int enabled;
    float enemy_health_multiplier;
    float enemy_speed_multiplier;
    float enemy_sight_multiplier;
    float enemy_damage_multiplier;
    int health_pickup_amount;
    int ammo_pickup_amount;
    int weapon_unlock_spare_magazines;
    int level_start_health_floor;
    int kill_score;
    int boss_max_health;
    int miniboss_max_health;
    int miniboss_support_count;
    int miniboss_wave_count;
    int boss_support_count;
    int boss_wave_count;
} DifficultyDef;

const DifficultyDef *difficulty_get(Difficulty d);

#endif
