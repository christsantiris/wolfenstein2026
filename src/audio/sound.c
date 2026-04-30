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
