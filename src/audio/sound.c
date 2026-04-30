#include "audio/sound.h"
#include <stdio.h>

int sound_load(Sound *s, const char *path) {
    s->chunk = Mix_LoadWAV(path);
    if (!s->chunk) {
        fprintf(stderr, "sound_load: %s: %s\n", path, Mix_GetError());
        return -1;
    }
    return 0;
}

void sound_play(const Sound *s) {
    if (s->chunk) {
        Mix_PlayChannel(-1, s->chunk, 0);
    }
}

void sound_free(Sound *s) {
    if (s->chunk) {
        Mix_FreeChunk(s->chunk);
        s->chunk = NULL;
    }
}

int music_load(Music *m, const char *path) {
    m->music = Mix_LoadMUS(path);
    if (!m->music) {
        fprintf(stderr, "music_load: %s: %s\n", path, Mix_GetError());
        return -1;
    }
    return 0;
}

void music_play(const Music *m) {
    if (m->music) {
        Mix_PlayMusic(m->music, -1);
    }
}

void music_stop(void) {
    Mix_HaltMusic();
}

void music_free(Music *m) {
    if (m->music) {
        Mix_FreeMusic(m->music);
        m->music = NULL;
    }
}
