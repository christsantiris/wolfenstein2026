#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#define HIGHSCORE_MAX 10
#define HIGHSCORE_FILE "highscores.dat"

typedef struct {
    int scores[HIGHSCORE_MAX];
    int count;
} HighScoreTable;

void highscore_load(HighScoreTable *t);
void highscore_save(const HighScoreTable *t);
int  highscore_insert(HighScoreTable *t, int score);

#endif
