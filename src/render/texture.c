#include "render/texture.h"
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

void texture_generate_door(Texture *t) {
    int border = 6;
    int mid = t->width / 2;

    for (int y = 0; y < t->height; y++) {
        for (int x = 0; x < t->width; x++) {
            int on_border = (x < border || x >= t->width - border ||
                             y < border || y >= t->height - border ||
                             (x >= mid - 2 && x < mid + 2));
            unsigned char r, g, b;
            if (on_border) {
                int vary = ((x * 5 + y * 11) % 14) - 7;
                r = (unsigned char)(90 + vary);
                g = (unsigned char)(55 + vary / 2);
                b = (unsigned char)(30 + vary / 2);
            } else {
                int vary = ((x * 3 + y * 7) % 18) - 9;
                r = (unsigned char)(160 + vary);
                g = (unsigned char)(100 + vary / 2);
                b = (unsigned char)(55  + vary / 2);
            }
            int panel_cx = (x < mid) ? (border + (mid - border) / 2) : (mid + (t->width - border - mid) / 2);
            int panel_cy = t->height * 2 / 3;
            int handle = (abs(x - panel_cx) <= 2 && abs(y - panel_cy) <= 2);
            if (handle) {
                r = 200; g = 170; b = 80;
            }
            int idx = (y * t->width + x) * 3;
            t->pixels[idx]     = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_exit_door(Texture *t) {
    int rivet_spacing = 16;
    int rivet_size    = 2;

    for (int y = 0; y < t->height; y++) {
        for (int x = 0; x < t->width; x++) {
            int vary = ((x * 7 + y * 3) % 12) - 6;
            unsigned char r = (unsigned char)(45 + vary);
            unsigned char g = (unsigned char)(15 + vary / 2);
            unsigned char b = (unsigned char)(15 + vary / 2);

            int rx = x % rivet_spacing;
            int ry = y % rivet_spacing;
            int on_rivet = (rx < rivet_size && ry < rivet_size);
            if (on_rivet) {
                r = 80; g = 80; b = 80;
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx]     = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
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
