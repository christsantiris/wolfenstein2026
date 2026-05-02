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

    /* muzzle */
    fill(29, 4, 34, 8, 25, 25, 25);

    /* barrel */
    fill(28, 7, 35, 21, 58, 58, 58);
    fill(28, 7, 28, 21, 78, 78, 78);    /* left highlight */
    fill(35, 7, 35, 21, 42, 42, 42);    /* right shadow */

    /* gas tube (sits above barrel, slightly narrower) */
    fill(29, 7, 33, 17, 72, 72, 72);
    fill(29, 7, 29, 17, 88, 88, 88);    /* left highlight */

    /* front sight post */
    fill(30, 19, 32, 22, 35, 35, 35);

    /* front wooden handguard */
    fill(23, 19, 40, 25, 105, 62, 22);
    fill(23, 19, 23, 25, 135, 82, 28);  /* left highlight */
    fill(40, 19, 40, 25, 78, 46, 14);   /* right shadow */
    fill(26, 21, 37, 21, 82, 46, 14);   /* wood grain */
    fill(26, 23, 37, 23, 82, 46, 14);

    /* receiver body */
    fill(15, 23, 47, 37, 44, 44, 48);
    fill(15, 23, 47, 24, 66, 66, 70);   /* top edge */
    fill(15, 36, 47, 37, 30, 30, 33);   /* bottom shadow */
    fill(15, 23, 15, 37, 58, 58, 62);   /* left edge */
    fill(47, 23, 47, 37, 28, 28, 32);   /* right edge */

    /* rear sight */
    fill(17, 23, 21, 27, 34, 34, 38);

    /* ejection port */
    fill(36, 27, 45, 34, 28, 28, 32);

    /* charging handle */
    fill(45, 24, 51, 28, 52, 52, 56);
    fill(45, 24, 51, 25, 68, 68, 72);   /* top highlight */

    /* selector lever */
    fill(44, 30, 50, 33, 38, 38, 42);

    /* pistol grip - wood */
    fill(30, 35, 42, 54, 98, 57, 18);
    fill(30, 35, 30, 54, 125, 74, 25);  /* left highlight */
    fill(42, 35, 42, 54, 72, 42, 12);   /* right shadow */
    fill(32, 39, 40, 39, 72, 40, 11);   /* grip lines */
    fill(32, 43, 40, 43, 72, 40, 11);
    fill(32, 47, 40, 47, 72, 40, 11);
    fill(32, 51, 40, 51, 72, 40, 11);

    /* banana magazine - upper straight section */
    fill(21, 35, 38, 43, 50, 50, 54);
    fill(21, 35, 21, 43, 68, 68, 72);   /* left highlight */
    fill(38, 35, 38, 43, 36, 36, 40);   /* right shadow */
    fill(23, 37, 36, 37, 40, 40, 44);   /* panel line */
    fill(23, 41, 36, 41, 40, 40, 44);

    /* magazine curves left */
    fill(19, 43, 36, 49, 48, 48, 52);
    fill(19, 43, 19, 49, 65, 65, 69);
    fill(36, 43, 36, 49, 34, 34, 38);

    /* lower curve */
    fill(17, 49, 33, 55, 46, 46, 50);
    fill(17, 49, 17, 55, 62, 62, 66);

    /* magazine tip */
    fill(19, 55, 30, 59, 43, 43, 47);
    fill(21, 59, 28, 61, 38, 38, 42);

    /* magazine release */
    fill(21, 35, 24, 38, 62, 62, 66);

    /* hand gripping the pistol grip */
    fill(14, 48, 29, 63, 210, 172, 128);  /* left of grip */
    fill(43, 48, 53, 63, 200, 162, 120);  /* right of grip */
    fill(14, 48, 53, 52, 205, 167, 124);  /* fingers across front */
    /* knuckle shadows */
    fill(17, 48, 19, 51, 175, 138, 100);
    fill(24, 48, 26, 51, 175, 138, 100);
    fill(31, 48, 33, 51, 175, 138, 100);
    fill(38, 48, 40, 51, 175, 138, 100);
    fill(45, 48, 47, 51, 175, 138, 100);
    /* thumb on left */
    fill(13, 40, 19, 50, 205, 167, 124);
    fill(11, 37, 15, 43, 195, 158, 116);

    FILE *f = fopen("assets/sprites/ak47.ppm", "wb");
    if (!f) {
        fprintf(stderr, "gen_ak47: cannot write assets/sprites/ak47.ppm\n");
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            fwrite(img[y][x], 1, 3, f);
        }
    }
    fclose(f);
    printf("wrote assets/sprites/ak47.ppm\n");
    return 0;
}
