#include <stdio.h>
#include <stdlib.h>

#define W 64
#define H 64

static unsigned char img[H][W][3];

static void set_px(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= W || y < 0 || y >= H) { return; }
    img[y][x][0] = r;
    img[y][x][1] = g;
    img[y][x][2] = b;
}

int main(void) {
    /* background - magenta color key */
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            img[y][x][0] = 255;
            img[y][x][1] = 0;
            img[y][x][2] = 255;
        }
    }

    int lcx = W * 26 / 64;
    int rcx = W * 38 / 64;
    int br = 4;
    int barrel_top = H * 4 / 64;
    int barrel_bot = H * 38 / 64;

    /* two barrel tubes */
    for (int y = barrel_top; y < barrel_bot; y++) {
        int taper = (barrel_bot - y) * br / (barrel_bot - barrel_top + 1);
        int lr = br - taper / 6;
        if (lr < 2) { lr = 2; }
        for (int x = lcx - lr; x <= lcx + lr; x++) {
            unsigned char edge = (abs(x - lcx) >= lr) ? 40 : 58;
            set_px(x, y, edge, edge, (unsigned char)(edge + 6));
        }
        for (int x = rcx - lr; x <= rcx + lr; x++) {
            unsigned char edge = (abs(x - rcx) >= lr) ? 40 : 58;
            set_px(x, y, edge, edge, (unsigned char)(edge + 6));
        }
    }

    /* dark muzzle openings */
    for (int y = barrel_top; y < barrel_top + 5; y++) {
        for (int dx = -3; dx <= 3; dx++) {
            set_px(lcx + dx, y, 18, 18, 20);
            set_px(rcx + dx, y, 18, 18, 20);
        }
    }

    /* receiver block */
    int rec_x0 = lcx - br - 3;
    int rec_x1 = rcx + br + 3;
    int rec_y0 = barrel_bot;
    int rec_y1 = H * 52 / 64;
    for (int y = rec_y0; y < rec_y1; y++) {
        for (int x = rec_x0; x <= rec_x1; x++) {
            int top_shade = (y == rec_y0) ? 70 : 52;
            set_px(x, y, (unsigned char)top_shade, (unsigned char)top_shade, (unsigned char)(top_shade + 5));
        }
    }

    /* wood grip */
    int grip_x0 = W * 28 / 64;
    int grip_x1 = W * 36 / 64;
    for (int y = rec_y1; y < H - 2; y++) {
        for (int x = grip_x0; x <= grip_x1; x++) {
            int vary = ((x + y) % 6) - 3;
            set_px(x, y, (unsigned char)(88 + vary), (unsigned char)(52 + vary), (unsigned char)(18 + vary));
        }
    }

    /* trigger guard */
    int tg_y = rec_y1 + 3;
    int tg_cx = W / 2;
    for (int dx = -5; dx <= 5; dx++) {
        set_px(tg_cx + dx, tg_y, 62, 62, 66);
    }

    FILE *f = fopen("assets/sprites/shotgun.ppm", "wb");
    if (!f) {
        fprintf(stderr, "gen_shotgun: cannot write assets/sprites/shotgun.ppm\n");
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            fwrite(img[y][x], 1, 3, f);
        }
    }
    fclose(f);
    printf("wrote assets/sprites/shotgun.ppm\n");
    return 0;
}
