#include "core/difficulty.h"

static const DifficultyDef DIFFICULTIES[DIFF_COUNT] = {
    [DIFF_CAN_I_PLAY_DADDY] = {
        .id = DIFF_CAN_I_PLAY_DADDY,
        .name = "CAN I PLAY, DADDY?",
        .enabled = 1,
        .enemy_health_multiplier = 0.75f,
        .enemy_speed_multiplier = 0.80f,
        .enemy_sight_multiplier = 0.80f,
        .enemy_damage_multiplier = 0.60f,
        .health_pickup_amount = 40,
        .ammo_pickup_amount = 12,
        .weapon_unlock_spare_magazines = 3,
        .level_start_health_floor = 100,
        .kill_score = 75,
        .boss_max_health = 1200,
        .miniboss_max_health = 712,
        .miniboss_support_count = 2,
        .miniboss_wave_count = 2,
        .boss_support_count = 3,
        .boss_wave_count = 2
    },
    [DIFF_DONT_HURT_ME] = {
        .id = DIFF_DONT_HURT_ME,
        .name = "DONT HURT ME.",
        .enabled = 1,
        .enemy_health_multiplier = 1.0f,
        .enemy_speed_multiplier = 1.0f,
        .enemy_sight_multiplier = 1.0f,
        .enemy_damage_multiplier = 1.0f,
        .health_pickup_amount = 25,
        .ammo_pickup_amount = 12,
        .weapon_unlock_spare_magazines = 2,
        .level_start_health_floor = 100,
        .kill_score = 100,
        .boss_max_health = 1600,
        .miniboss_max_health = 950,
        .miniboss_support_count = 3,
        .miniboss_wave_count = 3,
        .boss_support_count = 4,
        .boss_wave_count = 3
    },
    [DIFF_BRING_EM_ON] = {
        .id = DIFF_BRING_EM_ON,
        .name = "BRING EM ON!",
        .enabled = 1,
        .enemy_health_multiplier = 1.15f,
        .enemy_speed_multiplier = 1.15f,
        .enemy_sight_multiplier = 1.20f,
        .enemy_damage_multiplier = 1.35f,
        .health_pickup_amount = 15,
        .ammo_pickup_amount = 12,
        .weapon_unlock_spare_magazines = 1,
        .level_start_health_floor = 75,
        .kill_score = 150,
        .boss_max_health = 1840,
        .miniboss_max_health = 1092,
        .miniboss_support_count = 4,
        .miniboss_wave_count = 4,
        .boss_support_count = 5,
        .boss_wave_count = 4
    },
    [DIFF_I_AM_DEATH_INCARNATE] = {
        .id = DIFF_I_AM_DEATH_INCARNATE,
        .name = "I AM DEATH INCARNATE!",
        .enabled = 1,
        .enemy_health_multiplier = 1.30f,
        .enemy_speed_multiplier = 1.30f,
        .enemy_sight_multiplier = 1.40f,
        .enemy_damage_multiplier = 1.90f,
        .health_pickup_amount = 10,
        .ammo_pickup_amount = 12,
        .weapon_unlock_spare_magazines = 1,
        .level_start_health_floor = 50,
        .kill_score = 200,
        .boss_max_health = 2080,
        .miniboss_max_health = 1235,
        .miniboss_support_count = 5,
        .miniboss_wave_count = 5,
        .boss_support_count = 6,
        .boss_wave_count = 5
    }
};

const DifficultyDef *difficulty_get(Difficulty difficulty) {
    if (difficulty < 0 || difficulty >= DIFF_COUNT) {
        difficulty = DIFF_I_AM_DEATH_INCARNATE;
    }
    return &DIFFICULTIES[difficulty];
}
