#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h>

typedef struct {
    Mix_Chunk *chunk;
} Sound;

int  sound_load(Sound *s, const char *path);
void sound_play(const Sound *s);
void sound_free(Sound *s);
void sound_set_enabled(int enabled);

typedef struct {
    Mix_Music *music;
} Music;

int  music_load(Music *m, const char *path);
void music_play(const Music *m);
void music_stop(void);
void music_free(Music *m);

#endif
