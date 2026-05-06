#include <stdio.h>
#include <string.h>

#define W 1024
#define H 1024

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
    /* dark military background */
    fill(0, 0, W-1, H-1, 14, 20, 12);

    /* subtle vignette — darker corners */
    for (int y = 0; y < 200; y++) {
        unsigned char v = (unsigned char)(14 - y * 14 / 200);
        fill(0, y, W-1, y, v, v+6, v);
        fill(0, H-1-y, W-1, H-1-y, v, v+6, v);
    }

    /* ---- HELMET ---- */
    /* dome */
    fill(340, 55,  684, 185, 68, 76, 68);
    /* brim */
    fill(278, 175, 746, 218, 55, 62, 55);
    /* brim top highlight */
    fill(290, 175, 734, 182, 78, 88, 78);
    /* brim underside shadow */
    fill(285, 215, 739, 234, 32, 38, 32);
    /* dome highlight ridge */
    fill(360, 72,  450, 88,  90, 100, 90);
    /* helmet badge - gold */
    fill(490, 128, 534, 168, 185, 155, 55);
    fill(500, 120, 524, 130, 170, 140, 45);
    /* chin strap shadows on face sides */
    fill(340, 218, 375, 360, 52, 58, 52);
    fill(649, 218, 684, 360, 52, 58, 52);

    /* ---- FACE ---- */
    /* main face */
    fill(376, 218, 648, 500, 215, 188, 152);
    /* jaw sides (slightly darker) */
    fill(356, 340, 394, 504, 198, 172, 138);
    fill(630, 340, 668, 504, 198, 172, 138);
    /* forehead shadow under brim */
    fill(380, 218, 644, 248, 190, 165, 130);

    /* left eyebrow */
    fill(392, 256, 468, 272, 88, 68, 48);
    /* right eyebrow */
    fill(556, 256, 632, 272, 88, 68, 48);

    /* left eye socket */
    fill(388, 272, 472, 318, 58, 78, 118);
    /* left eye iris */
    fill(398, 278, 462, 312, 90, 130, 190);
    /* left pupil */
    fill(416, 284, 444, 306, 18, 38, 88);
    /* left eye white corners */
    fill(388, 278, 398, 312, 220, 220, 210);
    fill(462, 278, 472, 312, 220, 220, 210);

    /* right eye socket */
    fill(552, 272, 636, 318, 58, 78, 118);
    /* right eye iris */
    fill(562, 278, 626, 312, 90, 130, 190);
    /* right pupil */
    fill(580, 284, 608, 306, 18, 38, 88);
    /* right eye white corners */
    fill(552, 278, 562, 312, 220, 220, 210);
    fill(626, 278, 636, 312, 220, 220, 210);

    /* nose bridge */
    fill(498, 318, 526, 400, 188, 162, 128);
    /* nose tip */
    fill(482, 390, 542, 425, 195, 168, 134);
    /* nostril left */
    fill(486, 404, 500, 420, 155, 125, 96);
    /* nostril right */
    fill(524, 404, 538, 420, 155, 125, 96);

    /* upper lip */
    fill(422, 434, 602, 454, 148, 105, 74);
    /* lower lip */
    fill(430, 454, 594, 476, 175, 128, 92);
    /* lip highlight */
    fill(444, 456, 580, 466, 188, 140, 100);
    /* chin */
    fill(386, 472, 638, 510, 198, 172, 138);

    /* ---- NECK ---- */
    fill(456, 504, 568, 572, 205, 178, 142);

    /* ---- COLLAR / UNIFORM TOP ---- */
    /* collar flaps */
    fill(370, 560, 456, 618, 135, 105, 60);
    fill(568, 560, 654, 618, 135, 105, 60);
    /* collar centre V */
    fill(456, 560, 568, 618, 185, 148, 90);
    /* tie / shadow at V */
    fill(490, 572, 534, 618, 110, 85, 48);

    /* ---- UNIFORM BODY ---- */
    fill(292, 608, 732, 980, 185, 148, 90);
    /* left arm */
    fill(128, 614, 292, 920, 175, 140, 85);
    /* right arm */
    fill(732, 614, 896, 920, 175, 140, 85);

    /* shoulder epaulets */
    fill(292, 610, 390, 660, 158, 125, 72);
    fill(634, 610, 732, 660, 158, 125, 72);
    /* epaulet detail bars */
    fill(302, 624, 386, 632, 172, 140, 80);
    fill(638, 624, 722, 632, 172, 140, 80);

    /* centre chest shadow */
    fill(452, 628, 572, 800, 162, 128, 76);
    /* left chest pocket */
    fill(338, 648, 452, 752, 168, 134, 80);
    fill(344, 660, 446, 664, 148, 118, 66);
    /* right chest pocket */
    fill(572, 648, 686, 752, 168, 134, 80);
    fill(578, 660, 680, 664, 148, 118, 66);

    /* belt */
    fill(284, 820, 740, 862, 105, 82, 46);
    /* belt buckle */
    fill(482, 824, 542, 858, 175, 150, 70);
    fill(490, 830, 534, 852, 140, 115, 50);

    /* arm shadows */
    fill(128, 614, 165, 920, 155, 122, 72);
    fill(859, 614, 896, 920, 155, 122, 72);

    /* ---- GUN in right hand (viewer's left) ---- */
    /* forearm with gun */
    fill(128, 760, 240, 840, 175, 140, 85);
    /* pistol grip */
    fill(115, 758, 155, 836, 52, 42, 28);
    /* pistol frame / slide */
    fill(62,  740, 155, 770, 62, 62, 62);
    /* pistol barrel */
    fill(32,  748, 100, 762, 52, 52, 52);
    /* barrel highlight */
    fill(35,  749, 98,  756, 72, 72, 72);
    /* trigger guard */
    fill(120, 770, 145, 800, 62, 62, 62);
    /* trigger */
    fill(128, 776, 136, 792, 82, 82, 82);

    FILE *f = fopen("package/macos/icon.ppm", "wb");
    if (!f) {
        fprintf(stderr, "gen_icon: cannot write package/macos/icon.ppm\n");
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            fwrite(img[y][x], 1, 3, f);
        }
    }
    fclose(f);
    printf("wrote package/macos/icon.ppm\n");
    return 0;
}
