#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h>

typedef struct {
    Mix_Chunk *chunk;
} Sound;

int  sound_load(Sound *s, const char *path);
void sound_play(const Sound *s);
void sound_free(Sound *s);

#endif
