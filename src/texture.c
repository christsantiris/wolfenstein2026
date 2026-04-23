#include "texture.h"
#include <stdlib.h>
#include <stdio.h>

int texture_create(Texture *t, int width, int height) {
    t->pixels = malloc(width * height * 3);
    if (!t->pixels) {
        return -1;
    }
    t->width = width;
    t->height = height;
    return 0;
}

int texture_load_ppm(Texture *t, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "texture_load_ppm: cannot open %s\n", path);
        return -1;
    }

    char magic[3] = {0};
    int w = 0, h = 0, maxval = 0;

    if (fscanf(f, "%2s", magic) != 1 || magic[0] != 'P' || magic[1] != '6') {
        fprintf(stderr, "texture_load_ppm: not a P6 PPM\n");
        fclose(f);
        return -1;
    }

    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '#') {
            while ((c = fgetc(f)) != '\n' && c != EOF) {}
        } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            ungetc(c, f);
            break;
        }
    }

    if (fscanf(f, "%d %d %d", &w, &h, &maxval) != 3 || w <= 0 || h <= 0) {
        fprintf(stderr, "texture_load_ppm: invalid header in %s\n", path);
        fclose(f);
        return -1;
    }
    fgetc(f);

    if (texture_create(t, w, h) != 0) {
        fclose(f);
        return -1;
    }

    size_t want = (size_t)(w * h * 3);
    if (fread(t->pixels, 1, want, f) != want) {
        fprintf(stderr, "texture_load_ppm: short read in %s\n", path);
        texture_free(t);
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

void texture_free(Texture *t) {
    free(t->pixels);
    t->pixels = NULL;
    t->width = 0;
    t->height = 0;
}

unsigned int texture_sample(const Texture *t, float u, float v) {
    int x = (int)(u * t->width) % t->width;
    int y = (int)(v * t->height) % t->height;
    if (x < 0) { x += t->width; }
    if (y < 0) { y += t->height; }
    int idx = (y * t->width + x) * 3;
    return ((unsigned int)t->pixels[idx] << 16)
         | ((unsigned int)t->pixels[idx + 1] << 8)
         |  (unsigned int)t->pixels[idx + 2];
}

void texture_generate_brick(Texture *t) {
    int brick_w = 16;
    int brick_h = 8;
    int mortar  = 2;

    for (int y = 0; y < t->height; y++) {
        int row = y / brick_h;
        int offset = (row % 2) * (brick_w / 2);
        int mortar_y = (y % brick_h) < mortar;

        for (int x = 0; x < t->width; x++) {
            int col = (x + offset) % brick_w;
            int mortar_x = col < mortar;
            int in_mortar = mortar_y || mortar_x;

            unsigned char r, g, b;
            if (in_mortar) {
                r = 80; g = 75; b = 70;
            } else {
                int vary = ((x * 3 + y * 7) % 20) - 10;
                r = (unsigned char)(160 + vary);
                g = (unsigned char)(90  + vary / 2);
                b = (unsigned char)(70  + vary / 2);
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx]     = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}
