#include <stdio.h>
#include <string.h>

#define W 64
#define H 64

static unsigned char img[H][W][3];

static void fill(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x < 0 || x >= W || y < 0 || y >= H) { continue; }
            img[y][x][0] = r;
            img[y][x][1] = g;
            img[y][x][2] = b;
        }
    }
}

int main(void) {
    /* background - magenta color key */
    fill(0, 0, W-1, H-1, 255, 0, 255);

    /* helmet brim (wide) */
    fill(20, 6, 43, 8, 55, 62, 55);
    /* helmet dome */
    fill(23, 2, 40, 7, 65, 73, 65);
    /* helmet shadow underside */
    fill(21, 8, 42, 9, 40, 46, 40);

    /* face */
    fill(25, 9, 38, 19, 215, 188, 152);
    /* blue eyes */
    fill(27, 12, 29, 13, 90, 130, 190);
    fill(33, 12, 35, 13, 90, 130, 190);
    /* eye shadow */
    fill(27, 11, 29, 11, 60, 80, 120);
    fill(33, 11, 35, 11, 60, 80, 120);
    /* mouth */
    fill(29, 17, 34, 18, 170, 120, 88);
    /* chin */
    fill(26, 19, 37, 21, 200, 172, 138);

    /* neck */
    fill(29, 20, 33, 23, 205, 178, 142);

    /* collar */
    fill(25, 23, 38, 25, 140, 108, 62);

    /* uniform body - tan/khaki matching original */
    fill(22, 24, 41, 43, 185, 148, 90);
    /* left arm */
    fill(13, 25, 21, 42, 175, 140, 85);
    /* right arm */
    fill(42, 25, 50, 42, 175, 140, 85);
    /* chest shadow centre */
    fill(28, 27, 35, 38, 165, 130, 78);
    /* chest pocket left */
    fill(24, 28, 29, 33, 170, 135, 82);
    /* chest pocket right */
    fill(34, 28, 39, 33, 170, 135, 82);

    /* belt - dark */
    fill(22, 43, 41, 46, 42, 32, 18);
    /* belt buckle */
    fill(30, 43, 33, 46, 190, 162, 45);

    /* left leg */
    fill(22, 47, 30, 58, 175, 140, 84);
    /* right leg */
    fill(33, 47, 41, 58, 175, 140, 84);
    /* leg shadow */
    fill(25, 47, 28, 58, 155, 122, 72);
    fill(35, 47, 38, 58, 155, 122, 72);

    /* left boot */
    fill(21, 59, 31, 63, 28, 20, 12);
    /* right boot */
    fill(32, 59, 43, 63, 28, 20, 12);
    /* boot highlight */
    fill(23, 59, 25, 61, 45, 35, 22);
    fill(34, 59, 36, 61, 45, 35, 22);

    /* pistol in right hand */
    fill(42, 36, 56, 40, 62, 62, 62);
    fill(54, 37, 63, 39, 52, 52, 52);
    fill(43, 41, 48, 46, 50, 40, 25);

    FILE *f = fopen("assets/sprites/guard_front.ppm", "wb");
    if (!f) {
        fprintf(stderr, "gen_guard: cannot write assets/sprites/guard_front.ppm\n");
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            fwrite(img[y][x], 1, 3, f);
        }
    }
    fclose(f);
    printf("wrote assets/sprites/guard_front.ppm\n");
    return 0;
}
