#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
    unsigned char *pixels;
    int width;
    int height;
} Texture;

int texture_create(Texture *t, int width, int height);
void texture_free(Texture *t);
unsigned int texture_sample(const Texture *t, float u, float v);
void texture_generate_brick(Texture *t);

#endif
