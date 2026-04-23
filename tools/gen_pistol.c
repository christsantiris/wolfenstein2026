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

    /* barrel (pointing away — narrow at top, wider below) */
    fill(28, 6, 35, 12, 20, 20, 20);    /* muzzle opening */
    fill(27, 12, 36, 26, 60, 60, 60);   /* barrel body */
    fill(27, 12, 27, 26, 80, 80, 80);   /* left highlight */
    fill(36, 12, 36, 26, 45, 45, 45);   /* right shadow */

    /* slide — wider than barrel, viewed from slightly above */
    fill(20, 22, 43, 40, 72, 72, 72);
    fill(20, 22, 43, 23, 95, 95, 95);   /* top highlight */
    fill(20, 39, 43, 40, 50, 50, 50);   /* bottom shadow */
    fill(20, 22, 20, 40, 88, 88, 88);   /* left edge */
    fill(43, 22, 43, 40, 48, 48, 48);   /* right edge */

    /* front sight */
    fill(31, 20, 32, 23, 40, 40, 40);

    /* rear sight notch */
    fill(22, 22, 24, 25, 40, 40, 40);
    fill(38, 22, 41, 25, 40, 40, 40);

    /* ejection port */
    fill(30, 27, 41, 34, 55, 55, 55);

    /* trigger guard */
    fill(28, 40, 42, 44, 65, 65, 65);
    fill(28, 44, 30, 50, 65, 65, 65);
    fill(40, 44, 42, 50, 65, 65, 65);

    /* trigger */
    fill(33, 41, 35, 48, 40, 40, 40);

    /* grip — dark textured */
    fill(22, 40, 42, 62, 58, 40, 22);
    fill(22, 40, 22, 62, 72, 52, 28);   /* left highlight */
    fill(42, 40, 42, 62, 42, 28, 14);   /* right shadow */
    /* grip panel lines */
    fill(24, 45, 40, 45, 48, 32, 16);
    fill(24, 50, 40, 50, 48, 32, 16);
    fill(24, 55, 40, 55, 48, 32, 16);
    fill(24, 60, 40, 60, 48, 32, 16);

    /* hammer */
    fill(40, 35, 44, 41, 80, 80, 80);

    /* hand — skin tones gripping the lower half */
    fill(14, 46, 21, 63, 210, 172, 128); /* left of grip */
    fill(43, 46, 50, 63, 200, 162, 120); /* right of grip */
    fill(14, 46, 50, 50, 205, 167, 124); /* fingers across front */
    /* knuckle shadows */
    fill(16, 46, 18, 48, 175, 138, 100);
    fill(23, 46, 25, 48, 175, 138, 100);
    fill(30, 46, 32, 48, 175, 138, 100);
    fill(37, 46, 39, 48, 175, 138, 100);
    fill(44, 46, 46, 48, 175, 138, 100);
    /* thumb on left side */
    fill(14, 38, 20, 47, 205, 167, 124);
    fill(12, 36, 16, 42, 195, 158, 116);

    FILE *f = fopen("assets/sprites/pistol.ppm", "wb");
    if (!f) {
        fprintf(stderr, "gen_pistol: cannot write assets/sprites/pistol.ppm\n");
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            fwrite(img[y][x], 1, 3, f);
        }
    }
    fclose(f);
    printf("wrote assets/sprites/pistol.ppm\n");
    return 0;
}
