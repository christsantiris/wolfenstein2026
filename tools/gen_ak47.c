#include <stdio.h>
#include <string.h>

#define W 64
#define H 64

static unsigned char base[H][W][3];
static unsigned char frame[H][W][3];

static int load_ppm(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "gen_ak47: cannot read %s\n", path);
        return -1;
    }

    char magic[3] = { 0 };
    int width = 0;
    int height = 0;
    int max_value = 0;
    if (fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P6") != 0 || fscanf(f, "%d %d", &width, &height) != 2 || fscanf(f, "%d", &max_value) != 1) {
        fclose(f);
        fprintf(stderr, "gen_ak47: invalid PPM header in %s\n", path);
        return -1;
    }
    if (width != W || height != H || max_value != 255) {
        fclose(f);
        fprintf(stderr, "gen_ak47: expected a 64x64 PPM in %s\n", path);
        return -1;
    }
    int separator = fgetc(f);
    if (separator == '\r') {
        separator = fgetc(f);
    }
    if (separator != '\n' || fread(base, 1, sizeof(base), f) != sizeof(base)) {
        fclose(f);
        fprintf(stderr, "gen_ak47: incomplete pixel data in %s\n", path);
        return -1;
    }
    fclose(f);
    return 0;
}

static int write_ppm(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "gen_ak47: cannot write %s\n", path);
        return -1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    fwrite(frame, 1, sizeof(frame), f);
    fclose(f);
    return 0;
}

static void fill(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x < 0 || x >= W || y < 0 || y >= H) {
                continue;
            }
            frame[y][x][0] = r;
            frame[y][x][1] = g;
            frame[y][x][2] = b;
        }
    }
}

static void build_firing_frame(void) {
    fill(0, 0, W - 1, H - 1, 255, 0, 255);
    for (int y = 0; y < H; y++) {
        int shift_x = -(H - 1 - y) / 18;
        for (int x = 0; x < W; x++) {
            int source_x = x - shift_x;
            if (source_x < 0 || source_x >= W) {
                continue;
            }
            memcpy(frame[y][x], base[y][source_x], 3);
        }
    }

    fill(45, 37, 53, 43, 15, 15, 17);
    fill(49, 37, 55, 42, 78, 78, 82);

    fill(52, 27, 59, 29, 226, 178, 66);
    fill(57, 25, 62, 27, 252, 215, 108);
    fill(52, 30, 58, 30, 126, 83, 27);

    fill(23, 0, 25, 1, 255, 255, 210);
    fill(21, 2, 27, 4, 255, 226, 94);
    fill(19, 5, 29, 7, 255, 142, 24);
    fill(22, 2, 26, 7, 255, 246, 168);
}

int main(void) {
    if (load_ppm("assets/sprites/ak47.ppm") != 0) {
        return 1;
    }
    build_firing_frame();
    if (write_ppm("assets/sprites/ak47_fire.ppm") != 0) {
        return 1;
    }
    printf("wrote assets/sprites/ak47_fire.ppm\n");
    return 0;
}
