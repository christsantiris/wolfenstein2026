#include "ui/highscore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void highscore_path(char *path, int path_size) {
    const char *user_data_path = getenv("WOLF_USER_DATA_DIR");
    if (user_data_path) {
        snprintf(path, path_size, "%s%s", user_data_path, HIGHSCORE_FILE);
    } else {
        snprintf(path, path_size, "%s", HIGHSCORE_FILE);
    }
}

void highscore_load(HighScoreTable *t) {
    memset(t, 0, sizeof(*t));
    char path[1024];
    highscore_path(path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (!f) {
        return;
    }
    while (t->count < HIGHSCORE_MAX) {
        int s;
        if (fscanf(f, "%d", &s) != 1) {
            break;
        }
        t->scores[t->count++] = s;
    }
    fclose(f);
}

void highscore_save(const HighScoreTable *t) {
    char path[1024];
    highscore_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) {
        return;
    }
    for (int i = 0; i < t->count; i++) {
        fprintf(f, "%d\n", t->scores[i]);
    }
    fclose(f);
}

int highscore_insert(HighScoreTable *t, int score) {
    int rank = t->count;
    for (int i = 0; i < t->count; i++) {
        if (score > t->scores[i]) {
            rank = i;
            break;
        }
    }
    if (rank >= HIGHSCORE_MAX) {
        return -1;
    }
    int end = (t->count < HIGHSCORE_MAX) ? t->count : HIGHSCORE_MAX - 1;
    for (int i = end; i > rank; i--) {
        t->scores[i] = t->scores[i - 1];
    }
    t->scores[rank] = score;
    if (t->count < HIGHSCORE_MAX) {
        t->count++;
    }
    return rank + 1;
}
