#include "ui/highscore.h"
#include <stdio.h>
#include <string.h>

void highscore_load(HighScoreTable *t) {
    memset(t, 0, sizeof(*t));
    FILE *f = fopen(HIGHSCORE_FILE, "r");
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
    FILE *f = fopen(HIGHSCORE_FILE, "w");
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
