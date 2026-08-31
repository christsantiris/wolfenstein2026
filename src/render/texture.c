#include "render/texture.h"
#include <stdlib.h>
#include <stdio.h>

static int texture_is_magenta_key(const unsigned char *pixel) {
    return pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255;
}

static int texture_is_magenta_fringe(const unsigned char *pixel) {
    int r = pixel[0];
    int g = pixel[1];
    int b = pixel[2];
    return r >= 96 && b >= 64 && b > g * 3 / 2 && r > g * 3 / 2;
}

static void texture_remove_magenta_fringe(Texture *t) {
    int pixel_count = t->width * t->height;
    for (int i = 0; i < pixel_count; i++) {
        unsigned char *pixel = &t->pixels[i * 3];
        if (!texture_is_magenta_key(pixel) && texture_is_magenta_fringe(pixel)) {
            pixel[0] = 255;
            pixel[1] = 0;
            pixel[2] = 255;
        }
    }
}

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

    texture_remove_magenta_fringe(t);
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

typedef struct {
    int panel_r;
    int panel_g;
    int panel_b;
    int recess_r;
    int recess_g;
    int recess_b;
    int trim_r;
    int trim_g;
    int trim_b;
    int metal_r;
    int metal_g;
    int metal_b;
} DoorPalette;

static const DoorPalette DOOR_PALETTES[14] = {
    { 126, 54, 42, 92, 34, 28, 158, 82, 58, 72, 68, 62 },
    { 104, 108, 106, 70, 75, 75, 145, 148, 140, 68, 72, 72 },
    { 142, 112, 72, 100, 75, 48, 178, 145, 84, 82, 71, 55 },
    { 104, 98, 78, 68, 66, 55, 128, 120, 92, 64, 68, 66 },
    { 66, 86, 112, 43, 58, 80, 96, 124, 150, 56, 66, 78 },
    { 82, 88, 74, 52, 58, 47, 119, 126, 103, 63, 68, 61 },
    { 88, 62, 38, 55, 39, 26, 142, 105, 62, 70, 65, 56 },
    { 126, 54, 42, 92, 34, 28, 158, 82, 58, 72, 68, 62 },
    { 72, 78, 59, 48, 54, 40, 112, 116, 76, 61, 66, 63 },
    { 76, 87, 67, 47, 59, 46, 108, 119, 81, 62, 68, 64 },
    { 112, 58, 48, 76, 39, 34, 138, 82, 63, 66, 69, 70 },
    { 92, 101, 108, 58, 66, 73, 134, 145, 150, 62, 70, 76 },
    { 58, 65, 55, 36, 43, 38, 91, 99, 77, 50, 57, 55 },
    { 48, 43, 48, 27, 24, 29, 84, 70, 66, 45, 48, 52 }
};

static unsigned char door_colour(int value) {
    if (value < 0) {
        return 0;
    }
    if (value > 255) {
        return 255;
    }
    return (unsigned char)value;
}

static void generate_detailed_door(Texture *t, const DoorPalette *p, int exit_door, int seed) {
    int mid = t->width / 2;
    for (int y = 0; y < t->height; y++) {
        for (int x = 0; x < t->width; x++) {
            int noise = ((x * 17 + y * 29 + seed * 11) % 13) - 6;
            int r = p->panel_r + noise;
            int g = p->panel_g + noise;
            int b = p->panel_b + noise;

            int outer_frame = x < 7 || x >= t->width - 7 || y < 5 || y >= t->height - 5;
            int crossbar = y >= 29 && y <= 34;
            int seam = x >= mid - 2 && x <= mid + 1;
            if (outer_frame || crossbar || seam) {
                int frame_noise = ((x * 7 + y * 19 + seed * 5) % 11) - 5;
                r = p->metal_r + frame_noise;
                g = p->metal_g + frame_noise;
                b = p->metal_b + frame_noise;
            }

            int panel_x0 = x < mid ? 10 : 36;
            int panel_x1 = x < mid ? 27 : 53;
            int panel_y0 = y < mid ? 8 : 38;
            int panel_y1 = y < mid ? 25 : 55;
            int in_panel = x >= panel_x0 && x <= panel_x1 && y >= panel_y0 && y <= panel_y1;
            if (in_panel) {
                if (x == panel_x0 || y == panel_y0) {
                    r = p->trim_r + 18;
                    g = p->trim_g + 18;
                    b = p->trim_b + 18;
                } else if (x == panel_x1 || y == panel_y1) {
                    r = p->trim_r - 28;
                    g = p->trim_g - 28;
                    b = p->trim_b - 28;
                } else if (x == panel_x0 + 1 || y == panel_y0 + 1 || x == panel_x1 - 1 || y == panel_y1 - 1) {
                    r = p->trim_r;
                    g = p->trim_g;
                    b = p->trim_b;
                } else {
                    r = p->recess_r + noise;
                    g = p->recess_g + noise;
                    b = p->recess_b + noise;
                }
            }

            if (x == 7 || y == 5 || x == mid - 3) {
                r += 24;
                g += 24;
                b += 24;
            }
            if (x == t->width - 8 || y == t->height - 6 || x == mid + 2) {
                r -= 24;
                g -= 24;
                b -= 24;
            }

            int lock_plate = x >= mid + 1 && x <= mid + 9 && y >= 26 && y <= 43;
            if (lock_plate) {
                int plate_edge = x == mid + 1 || x == mid + 9 || y == 26 || y == 43;
                r = p->trim_r + (plate_edge ? 18 : -8);
                g = p->trim_g + (plate_edge ? 18 : -8);
                b = p->trim_b + (plate_edge ? 18 : -8);
            }

            int handle = x >= mid + 7 && x <= mid + 18 && y >= 31 && y <= 34;
            if (handle) {
                r = exit_door ? 205 : 172;
                g = exit_door ? 151 : 139;
                b = exit_door ? 66 : 86;
                if (y == 34) {
                    r -= 45;
                    g -= 45;
                    b -= 35;
                }
            }
            int keyhole = x >= mid + 4 && x <= mid + 6 && y >= 37 && y <= 41;
            if (keyhole) {
                r = 18;
                g = 17;
                b = 16;
            }

            int rivet_column = (x >= 2 && x <= 4) || (x >= t->width - 5 && x <= t->width - 3);
            int rivet_row = y == 9 || y == 21 || y == 33 || y == 45 || y == 56;
            if (rivet_column && rivet_row) {
                int rivet_highlight = x == 2 || x == t->width - 5;
                r = rivet_highlight ? 154 : 54;
                g = rivet_highlight ? 151 : 53;
                b = rivet_highlight ? 139 : 51;
            }

            int worn = (x * 23 + y * 37 + seed * 41) % 211 == 0;
            if (worn && !keyhole) {
                r += 34;
                g += 30;
                b += 24;
            }
            int grime = y > 48 && (x * 11 + y * 5 + seed) % 17 < 3;
            if (grime) {
                r -= 16;
                g -= 16;
                b -= 14;
            }

            if (exit_door && (x == 7 || x == t->width - 8 || y == 5)) {
                r = 184;
                g = 133;
                b = 52;
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = door_colour(r);
            t->pixels[idx + 1] = door_colour(g);
            t->pixels[idx + 2] = door_colour(b);
        }
    }
}

void texture_generate_door(Texture *t, int level) {
    int palette = level - 1;
    if (palette < 0 || palette >= 14) {
        palette = 0;
    }
    generate_detailed_door(t, &DOOR_PALETTES[palette], 0, level);
}

void texture_generate_exit_door(Texture *t) {
    int rivet_spacing = 16;
    int rivet_size = 2;

    for (int y = 0; y < t->height; y++) {
        for (int x = 0; x < t->width; x++) {
            int vary = ((x * 7 + y * 3) % 12) - 6;
            unsigned char r = (unsigned char)(45 + vary);
            unsigned char g = (unsigned char)(15 + vary / 2);
            unsigned char b = (unsigned char)(15 + vary / 2);

            int rx = x % rivet_spacing;
            int ry = y % rivet_spacing;
            int on_rivet = rx < rivet_size && ry < rivet_size;
            if (on_rivet) {
                r = 80;
                g = 80;
                b = 80;
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_guard_dir(Texture *t, int dir) {
    int W = t->width;
    int H = t->height;

    /* dir 0=back, 1=back-right, 2=right, 3=front-right,
           4=front, 5=front-left, 6=left, 7=back-left     */
    static const int BODY_W[8]    = { 34, 28, 18, 28, 36, 28, 18, 28 };
    static const int SHOW_FACE[8] = {  0,  0,  0,  1,  1,  1,  0,  0 };

    int body_w  = BODY_W[dir]  * W / 64;
    int show_face = SHOW_FACE[dir];

    int cx = W / 2;

    /* magenta background */
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i]     = 255;
        t->pixels[i + 1] = 0;
        t->pixels[i + 2] = 255;
    }

    int bx0 = cx - body_w / 2;
    int bx1 = cx + body_w / 2;

    /* boots — two separate legs */
    int boot_y0 = H * 50 / 64;
    int leg_w   = body_w / 3;
    int leg_gap = body_w / 6;
    int llx0 = cx - leg_w - leg_gap / 2;
    int llx1 = cx - leg_gap / 2;
    int lrx0 = cx + leg_gap / 2;
    int lrx1 = cx + leg_w + leg_gap / 2;
    for (int y = boot_y0; y < H; y++) {
        for (int x = llx0; x < llx1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 90; t->pixels[idx + 1] = 50; t->pixels[idx + 2] = 20;
        }
        for (int x = lrx0; x < lrx1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 90; t->pixels[idx + 1] = 50; t->pixels[idx + 2] = 20;
        }
    }

    /* uniform body — khaki */
    int body_y0 = H * 26 / 64;
    for (int y = body_y0; y < boot_y0; y++) {
        for (int x = bx0; x < bx1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 200; t->pixels[idx + 1] = 160; t->pixels[idx + 2] = 80;
        }
    }

    /* belt stripe */
    int belt_y = H * 42 / 64;
    for (int x = bx0; x < bx1; x++) {
        int idx = (belt_y * W + x) * 3;
        t->pixels[idx] = 100; t->pixels[idx + 1] = 70; t->pixels[idx + 2] = 20;
    }

    /* head — warm skin */
    int head_w  = body_w * 6 / 10;
    int hx0 = cx - head_w / 2;
    int hx1 = cx + head_w / 2;
    int head_y0 = H * 10 / 64;
    int head_y1 = body_y0;
    unsigned char hr = show_face ? 220 : 170;
    unsigned char hg = show_face ? 180 : 130;
    unsigned char hb = show_face ? 130 :  90;
    for (int y = head_y0; y < head_y1; y++) {
        for (int x = hx0; x < hx1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = hr; t->pixels[idx + 1] = hg; t->pixels[idx + 2] = hb;
        }
    }

    /* cap — dark olive */
    int cap_y0 = H * 5 / 64;
    int cap_x0 = hx0 - W / 16;
    int cap_x1 = hx1 + W / 16;
    if (cap_x0 < 0) { cap_x0 = 0; }
    if (cap_x1 > W) { cap_x1 = W; }
    for (int y = cap_y0; y < head_y0 + 2; y++) {
        for (int x = cap_x0; x < cap_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 55; t->pixels[idx + 1] = 65; t->pixels[idx + 2] = 38;
        }
    }

    /* eyes — blue for front-facing */
    if (show_face && head_w >= 6) {
        int ey = head_y0 + (head_y1 - head_y0) / 2;
        int ex_l = cx - head_w / 4;
        int ex_r = cx + head_w / 4;
        for (int dx = 0; dx < 2; dx++) {
            for (int dy2 = 0; dy2 < 2; dy2++) {
                int il = ((ey + dy2) * W + ex_l + dx) * 3;
                int ir = ((ey + dy2) * W + ex_r + dx) * 3;
                t->pixels[il]     = 70; t->pixels[il + 1] = 120; t->pixels[il + 2] = 200;
                t->pixels[ir]     = 70; t->pixels[ir + 1] = 120; t->pixels[ir + 2] = 200;
            }
        }
    }

    /* gun arm — held at hip, extends right of body */
    int arm_y0 = H * 35 / 64;
    int arm_y1 = H * 43 / 64;
    int arm_x0 = cx;
    int arm_x1 = bx1 + W * 10 / 64;
    if (arm_x1 > W) { arm_x1 = W; }
    for (int y = arm_y0; y < arm_y1; y++) {
        for (int x = arm_x0; x < arm_x1; x++) {
            int idx = (y * W + x) * 3;
            if (t->pixels[idx] == 255 && t->pixels[idx + 1] == 0 && t->pixels[idx + 2] == 255) {
                /* arm: uniform colour */
                t->pixels[idx] = 200; t->pixels[idx + 1] = 160; t->pixels[idx + 2] = 80;
            }
        }
    }
    /* gun — darker grey block at end of arm */
    int gun_x0 = bx1;
    int gun_x1 = arm_x1;
    int gun_y0 = H * 37 / 64;
    int gun_y1 = H * 41 / 64;
    for (int y = gun_y0; y < gun_y1; y++) {
        for (int x = gun_x0; x < gun_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 54; t->pixels[idx + 1] = 54; t->pixels[idx + 2] = 62;
        }
    }
}

void texture_generate_guard_walk_b(Texture *t, int dir) {
    (void)dir;
    int W = t->width;
    int H = t->height;
    int bob = 2;
    for (int y = 0; y < H - bob; y++) {
        for (int x = 0; x < W; x++) {
            int di = (y * W + x) * 3;
            int si = ((y + bob) * W + x) * 3;
            t->pixels[di] = t->pixels[si];
            t->pixels[di+1] = t->pixels[si+1];
            t->pixels[di+2] = t->pixels[si+2];
        }
    }
    for (int y = H - bob; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            t->pixels[i] = 255; t->pixels[i+1] = 0; t->pixels[i+2] = 255;
        }
    }
}

void texture_generate_guard_attack(Texture *t) {
    texture_generate_guard_dir(t, 4);
    int W = t->width;
    int H = t->height;
    int cx = W / 2;

    /* arm raised — uniform-colored block from torso centre upward */
    int arm_x0 = cx - W * 3 / 64;
    int arm_x1 = cx + W * 3 / 64;
    for (int y = H * 30 / 64; y < H * 42 / 64; y++) {
        for (int x = arm_x0; x < arm_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 200; t->pixels[idx+1] = 160; t->pixels[idx+2] = 80;
        }
    }
    /* gun slide — grey rectangle at arm end */
    int gun_x0 = cx - W * 5 / 64;
    int gun_x1 = cx + W * 5 / 64;
    for (int y = H * 22 / 64; y < H * 32 / 64; y++) {
        for (int x = gun_x0; x < gun_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 54; t->pixels[idx+1] = 54; t->pixels[idx+2] = 62;
        }
    }
    /* barrel bore — dark centre square, gun pointing at viewer */
    int bore_x0 = cx - W * 2 / 64;
    int bore_x1 = cx + W * 2 / 64;
    for (int y = H * 18 / 64; y < H * 24 / 64; y++) {
        for (int x = bore_x0; x < bore_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 12; t->pixels[idx+1] = 12; t->pixels[idx+2] = 14;
        }
    }
}

void texture_generate_muzzle_flash(Texture *t, int cx, int cy, int radius) {
    int reach = radius + 2;
    for (int y = cy - reach; y <= cy + reach; y++) {
        for (int x = cx - reach; x <= cx + reach; x++) {
            if (x < 0 || x >= t->width || y < 0 || y >= t->height) {
                continue;
            }
            int dx = abs(x - cx);
            int dy = abs(y - cy);
            int diamond = dx + dy <= radius;
            int axial = (dx <= 1 && dy <= reach) || (dy <= 1 && dx <= reach);
            if (!diamond && !axial) {
                continue;
            }

            unsigned char r = 240;
            unsigned char g = 82;
            unsigned char b = 18;
            if (dx + dy <= radius / 2) {
                r = 255;
                g = 248;
                b = 190;
            } else if (diamond) {
                r = 255;
                g = 190;
                b = 38;
            }
            int index = (y * t->width + x) * 3;
            t->pixels[index] = r;
            t->pixels[index + 1] = g;
            t->pixels[index + 2] = b;
        }
    }
}

void texture_generate_boss_dir(Texture *t, int dir) {
    int W = t->width;
    int H = t->height;
    static const int BODY_W[8]    = { 46, 38, 28, 38, 48, 38, 28, 38 };
    static const int SHOW_FACE[8] = {  0,  0,  0,  1,  1,  1,  0,  0 };

    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255; t->pixels[i + 1] = 0; t->pixels[i + 2] = 255;
    }

    int body_w = BODY_W[dir] * W / 64;
    int show_face = SHOW_FACE[dir];
    int cx = W / 2;
    int bx0 = cx - body_w / 2;
    int bx1 = cx + body_w / 2;
    int coat_y0 = H * 24 / 64;
    int boot_y0 = H * 52 / 64;

    for (int y = boot_y0; y < H; y++) {
        for (int x = bx0 + body_w / 8; x < bx1 - body_w / 8; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 18; t->pixels[idx + 1] = 18; t->pixels[idx + 2] = 22;
        }
    }

    for (int y = coat_y0; y < boot_y0; y++) {
        int taper = (y - coat_y0) * body_w / (H * 80 / 64);
        int x0 = bx0 + taper / 6;
        int x1 = bx1 - taper / 6;
        for (int x = x0; x < x1; x++) {
            int shade = ((x * 5 + y * 7) % 12) - 6;
            int idx = (y * W + x) * 3;
            t->pixels[idx] = (unsigned char)(28 + shade / 2);
            t->pixels[idx + 1] = (unsigned char)(30 + shade / 2);
            t->pixels[idx + 2] = (unsigned char)(38 + shade);
        }
    }

    int sash_x = show_face ? cx - body_w / 5 : cx + body_w / 8;
    for (int y = coat_y0 + 2; y < boot_y0 - 2; y++) {
        int x = sash_x + (y - coat_y0) / 5;
        for (int dx = -2; dx <= 2; dx++) {
            int px = x + dx;
            if (px < bx0 || px >= bx1) { continue; }
            int idx = (y * W + px) * 3;
            t->pixels[idx] = 170; t->pixels[idx + 1] = 18; t->pixels[idx + 2] = 24;
        }
    }

    int belt_y = H * 42 / 64;
    for (int x = bx0; x < bx1; x++) {
        int idx = (belt_y * W + x) * 3;
        t->pixels[idx] = 12; t->pixels[idx + 1] = 12; t->pixels[idx + 2] = 14;
    }

    int gun_y = H * 36 / 64;
    int gun_x0 = show_face ? cx - body_w / 5 : cx - body_w / 8;
    int gun_x1 = show_face ? bx1 + W / 7 : bx1 + W / 10;
    if (dir == 5 || dir == 6 || dir == 7) {
        gun_x0 = bx0 - W / 10;
        gun_x1 = show_face ? cx + body_w / 5 : cx + body_w / 8;
    }
    if (gun_x0 < 0) { gun_x0 = 0; }
    if (gun_x1 > W) { gun_x1 = W; }
    for (int y = gun_y; y < gun_y + H / 14; y++) {
        for (int x = gun_x0; x < gun_x1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 50; t->pixels[idx + 1] = 50; t->pixels[idx + 2] = 56;
        }
    }
    int muzzle_x = (dir == 5 || dir == 6 || dir == 7) ? gun_x0 : gun_x1 - 2;
    for (int y = gun_y - 1; y < gun_y + H / 14 + 1; y++) {
        for (int x = muzzle_x; x < muzzle_x + 2 && x < W; x++) {
            if (x < 0 || y < 0 || y >= H) { continue; }
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 18; t->pixels[idx + 1] = 18; t->pixels[idx + 2] = 22;
        }
    }
    int grip_x0 = cx - body_w / 10;
    int grip_x1 = grip_x0 + W / 10;
    for (int y = gun_y + H / 14; y < gun_y + H / 5; y++) {
        for (int x = grip_x0; x < grip_x1; x++) {
            if (x < 0 || x >= W || y < 0 || y >= H) { continue; }
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 42; t->pixels[idx + 1] = 24; t->pixels[idx + 2] = 18;
        }
    }

    int head_w = body_w * 46 / 100;
    int hx0 = cx - head_w / 2;
    int hx1 = cx + head_w / 2;
    int head_y0 = H * 10 / 64;
    int head_y1 = coat_y0 + 2;
    unsigned char hr = show_face ? 218 : 115;
    unsigned char hg = show_face ? 190 : 100;
    unsigned char hb = show_face ? 160 :  88;
    for (int y = head_y0; y < head_y1; y++) {
        for (int x = hx0; x < hx1; x++) {
            int idx = (y * W + x) * 3;
            t->pixels[idx] = hr; t->pixels[idx + 1] = hg; t->pixels[idx + 2] = hb;
        }
    }

    int cap_y0 = H * 5 / 64;
    for (int y = cap_y0; y < head_y0 + 3; y++) {
        for (int x = hx0 - W / 12; x < hx1 + W / 12; x++) {
            if (x < 0 || x >= W) { continue; }
            int idx = (y * W + x) * 3;
            t->pixels[idx] = 18; t->pixels[idx + 1] = 18; t->pixels[idx + 2] = 22;
        }
    }

    if (show_face && head_w >= 8) {
        int ey = head_y0 + (head_y1 - head_y0) / 2;
        int ex_l = cx - head_w / 4;
        int ex_r = cx + head_w / 4;
        for (int dx = 0; dx < 2; dx++) {
            for (int dy = 0; dy < 2; dy++) {
                int il = ((ey + dy) * W + ex_l + dx) * 3;
                int ir = ((ey + dy) * W + ex_r + dx) * 3;
                t->pixels[il] = 190; t->pixels[il + 1] = 20; t->pixels[il + 2] = 24;
                t->pixels[ir] = 190; t->pixels[ir + 1] = 20; t->pixels[ir + 2] = 24;
            }
        }
    }
}

int texture_recolor_uniform(Texture *dst, const Texture *src, unsigned char ur, unsigned char ug, unsigned char ub) {
    int n = src->width * src->height;
    if (dst != src) {
        if (texture_create(dst, src->width, src->height) != 0) {
            return -1;
        }
    }
    for (int i = 0; i < n; i++) {
        unsigned char r = src->pixels[i * 3];
        unsigned char g = src->pixels[i * 3 + 1];
        unsigned char b = src->pixels[i * 3 + 2];
        if (r >= 150 && g >= 100 && g < r && b < g && b < 110) {
            float lum = r / 200.0f;
            int nr = (int)(ur * lum); if (nr > 255) { nr = 255; }
            int ng = (int)(ug * lum); if (ng > 255) { ng = 255; }
            int nb = (int)(ub * lum); if (nb > 255) { nb = 255; }
            dst->pixels[i * 3]     = (unsigned char)nr;
            dst->pixels[i * 3 + 1] = (unsigned char)ng;
            dst->pixels[i * 3 + 2] = (unsigned char)nb;
        } else if (dst != src) {
            dst->pixels[i * 3]     = r;
            dst->pixels[i * 3 + 1] = g;
            dst->pixels[i * 3 + 2] = b;
        }
    }
    return 0;
}

void texture_derive_guard_dirs(Texture guard_tex[8]) {
    const Texture *src = &guard_tex[4];
    int W = src->width;
    int H = src->height;

    /* body_pct: apparent width as % of full width (side=narrow, front/back=wide)
       is_back:  darken face region (player sees enemy's back)
       mirror:   flip source x (left-side views are mirror of right-side) */
    static const int BODY_PCT[8] = { 88, 65, 38, 65,  0, 65, 38, 65 };
    static const int IS_BACK[8]  = {  1,  1,  0,  0,  0,  0,  0,  1 };
    static const int MIRROR[8]   = {  0,  0,  0,  0,  0,  1,  1,  1 };

    for (int d = 0; d < 8; d++) {
        if (d == 4) { continue; }

        int pct     = BODY_PCT[d];
        int is_back = IS_BACK[d];
        int do_mirror = MIRROR[d];
        Texture *dst = &guard_tex[d];

        for (int i = 0; i < W * H * 3; i += 3) {
            dst->pixels[i] = 255; dst->pixels[i + 1] = 0; dst->pixels[i + 2] = 255;
        }

        int body_w  = W * pct / 100;
        int x_start = (W - body_w) / 2;
        float sx0   = (float)(W - body_w) / 2.0f;
        float sx1   = sx0 + (float)body_w;

        for (int out_y = 0; out_y < H; out_y++) {
            for (int bx = 0; bx < body_w; bx++) {
                float t = (body_w > 1) ? (float)bx / (float)(body_w - 1) : 0.5f;
                if (do_mirror) { t = 1.0f - t; }

                int sx = (int)(sx0 + t * (sx1 - sx0));
                if (sx < 0) { sx = 0; }
                if (sx >= W) { sx = W - 1; }

                int si = (out_y * W + sx) * 3;
                unsigned char r = src->pixels[si];
                unsigned char g = src->pixels[si + 1];
                unsigned char b = src->pixels[si + 2];
                if (r == 255 && g == 0 && b == 255) { continue; }

                if (is_back && out_y < H * 38 / 100) {
                    r = (unsigned char)(r * 2 / 3);
                    g = (unsigned char)(g * 2 / 3);
                    b = (unsigned char)(b * 2 / 3);
                }

                int di = (out_y * W + x_start + bx) * 3;
                dst->pixels[di]     = r;
                dst->pixels[di + 1] = g;
                dst->pixels[di + 2] = b;
            }
        }
    }
}

static void set_px(Texture *t, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= t->width || y < 0 || y >= t->height) { return; }
    int idx = (y * t->width + x) * 3;
    t->pixels[idx] = r; t->pixels[idx + 1] = g; t->pixels[idx + 2] = b;
}

void texture_generate_ammo_pickup(Texture *t) {
    int W = t->width;
    int H = t->height;
    /* magenta background */
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255; t->pixels[i + 1] = 0; t->pixels[i + 2] = 255;
    }
    /* draw two bullets side by side, vertically oriented */
    int offsets[2] = { W * 11 / 32, W * 20 / 32 };
    int bw = W * 5 / 32;
    int body_top = H * 28 / 64;
    int body_bot = H * 56 / 64;
    int tip_top  = H * 14 / 64;
    for (int b = 0; b < 2; b++) {
        int bx = offsets[b];
        /* brass body */
        for (int y = body_top; y <= body_bot; y++) {
            for (int x = bx; x < bx + bw; x++) {
                int shade = ((x - bx) == 1) ? 20 : 0;
                set_px(t, x, y, 190 + shade, 148 + shade, 42);
            }
        }
        /* silver tapering tip */
        for (int y = tip_top; y < body_top; y++) {
            int dist = body_top - y;
            int half  = bw / 2;
            int shrink = (dist * half) / (body_top - tip_top);
            if (shrink > half) { shrink = half; }
            for (int x = bx + shrink; x < bx + bw - shrink; x++) {
                unsigned char lum = (unsigned char)(160 + (bx + bw / 2 - x) * 8);
                set_px(t, x, y, lum, lum, lum);
            }
        }
        /* base rim */
        for (int x = bx; x < bx + bw; x++) {
            set_px(t, x, body_bot,     80, 60, 20);
            set_px(t, x, body_bot - 1, 80, 60, 20);
        }
    }
}

void texture_generate_health_pickup(Texture *t) {
    int W = t->width;
    int H = t->height;
    /* white background */
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 240; t->pixels[i + 1] = 240; t->pixels[i + 2] = 240;
    }
    /* red cross */
    int cx = W / 2;
    int cy = H / 2;
    int arm_w = W / 5;
    int arm_l = W * 2 / 5;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int in_h = (abs(y - cy) <= arm_w && abs(x - cx) <= arm_l);
            int in_v = (abs(x - cx) <= arm_w && abs(y - cy) <= arm_l);
            if (in_h || in_v) {
                set_px(t, x, y, 200, 30, 30);
            }
        }
    }
    /* thin border */
    for (int x = 0; x < W; x++) { set_px(t, x, 0, 180, 180, 180); set_px(t, x, H-1, 180, 180, 180); }
    for (int y = 0; y < H; y++) { set_px(t, y, 0, 180, 180, 180); set_px(t, W-1, y, 180, 180, 180); }
}

void texture_generate_wood(Texture *t) {
    int W = t->width;
    int H = t->height;
    int plank_w = 16;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int px = x % plank_w;
            int is_seam = (px == 0 || px == plank_w - 1);
            int grain = (x * 7 + x * x / 8) % 16;
            int is_grain = (grain < 2);
            int vary = ((x * 3 + y * 11) % 18) - 9;

            unsigned char r, g, b;
            if (is_seam) {
                r = 82; g = 46; b = 16;
            } else if (is_grain) {
                r = (unsigned char)(108 + vary / 2);
                g = (unsigned char)(62 + vary / 3);
                b = (unsigned char)(22 + vary / 4);
            } else {
                r = (unsigned char)(150 + vary);
                g = (unsigned char)(88 + vary * 2 / 3);
                b = (unsigned char)(36 + vary / 3);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_moss_stone(Texture *t) {
    int W = t->width;
    int H = t->height;
    int brick_w = 16;
    int brick_h = 8;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int row = y / brick_h;
            int col_offset = (row % 2) ? brick_w / 2 : 0;
            int bx = (x + col_offset) % brick_w;
            int by = y % brick_h;
            int is_mortar = (bx == 0 || by == 0);
            int brick_col = (x + col_offset) / brick_w;
            int is_moss = ((row * 3 + brick_col * 7) % 5 < 2);

            unsigned char r, g, b;
            if (is_mortar) {
                r = 48; g = 44; b = 30;
            } else if (is_moss) {
                int vary = ((bx * 5 + by * 11) % 18) - 9;
                r = (unsigned char)(62 + vary / 2);
                g = (unsigned char)(108 + vary);
                b = (unsigned char)(38 + vary / 2);
            } else {
                int vary = ((bx * 3 + by * 7 + row * 13) % 24) - 12;
                r = (unsigned char)(192 + vary);
                g = (unsigned char)(172 + vary * 3 / 4);
                b = (unsigned char)(98 + vary / 2);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_blue_brick(Texture *t) {
    int brick_w = 16;
    int brick_h = 8;
    int mortar = 2;

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
                r = 12; g = 16; b = 42;
            } else {
                int vary = ((x * 3 + y * 7) % 20) - 10;
                r = (unsigned char)(26 + vary / 2);
                g = (unsigned char)(38 + vary / 2);
                b = (unsigned char)(105 + vary);
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_red_blue_brick(Texture *t) {
    int W = t->width;
    int H = t->height;
    int brick_w = 16;
    int brick_h = 8;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int row = y / brick_h;
            int col_offset = (row % 2) ? brick_w / 2 : 0;
            int bx = (x + col_offset) % brick_w;
            int by = y % brick_h;
            int is_mortar = (bx == 0 || by == 0);
            int brick_col = (x + col_offset) / brick_w;
            int is_blue = ((row + brick_col) % 2);

            unsigned char r, g, b;
            if (is_mortar) {
                r = 44; g = 24; b = 54;
            } else if (is_blue) {
                int vary = ((bx * 3 + by * 7 + row * 11) % 20) - 10;
                r = (unsigned char)(26 + vary / 2);
                g = (unsigned char)(38 + vary / 2);
                b = (unsigned char)(105 + vary);
            } else {
                int vary = ((bx * 3 + by * 7 + row * 11) % 20) - 10;
                r = (unsigned char)(160 + vary);
                g = (unsigned char)(90 + vary / 2);
                b = (unsigned char)(70 + vary / 2);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_metal_panels(Texture *t) {
    int W = t->width;
    int H = t->height;
    int panel_w = 16;
    int panel_h = 16;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int px = x % panel_w;
            int py = y % panel_h;
            int is_seam = (px == 0 || py == 0);
            int is_rivet = ((px == 3 || px == panel_w - 4) &&
                            (py == 3 || py == panel_h - 4));
            int shine = (panel_w - px) / 4 + py / 8;
            int vary = ((x * 5 + y * 9) % 18) - 9;

            unsigned char r, g, b;
            if (is_seam) {
                r = 32; g = 36; b = 42;
            } else if (is_rivet) {
                r = 112; g = 120; b = 132;
            } else {
                int base = 72 + shine + vary;
                if (base < 45) { base = 45; }
                if (base > 130) { base = 130; }
                r = (unsigned char)base;
                g = (unsigned char)(base + 5);
                b = (unsigned char)(base + 12);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_command_bunker(Texture *t) {
    int W = t->width;
    int H = t->height;
    int slab_w = 32;
    int slab_h = 16;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int sx = x % slab_w;
            int sy = y % slab_h;
            int is_seam = (sx == 0 || sy == 0);
            int is_red_trim = (sy == 2 || sy == 3);
            int vein = ((x * 7 + y * 11 + (x * y) / 9) % 37 == 0);
            int vein2 = ((x * 5 - y * 3 + 256) % 29 == 0 && sy > 5);
            int vary = ((x * 3 + y * 13) % 16) - 8;

            unsigned char r, g, b;
            if (is_seam) {
                r = 18; g = 18; b = 22;
            } else if (is_red_trim) {
                int glow = ((x * 5 + y * 3) % 18) - 9;
                r = (unsigned char)(124 + glow);
                g = (unsigned char)(18 + glow / 3);
                b = (unsigned char)(22 + glow / 3);
            } else if (vein || vein2) {
                r = 82; g = 78; b = 84;
            } else {
                int base = 42 + vary;
                if (base < 24) { base = 24; }
                if (base > 62) { base = 62; }
                r = (unsigned char)base;
                g = (unsigned char)base;
                b = (unsigned char)(base + 5);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_obsidian_command(Texture *t) {
    int W = t->width;
    int H = t->height;
    int slab_w = 16;
    int slab_h = 16;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int sx = x % slab_w;
            int sy = y % slab_h;
            int is_seam = (sx == 0 || sy == 0);
            int is_red_channel = (sx == 2 || sx == 3);
            int crack = ((x * 9 + y * 5 + (x * y) / 7) % 41 == 0);
            int shine = ((x + y * 2) % 23 == 0);
            int vary = ((x * 11 + y * 7) % 14) - 7;

            unsigned char r, g, b;
            if (is_seam) {
                r = 10; g = 8; b = 14;
            } else if (is_red_channel) {
                int glow = ((y * 5 + x * 3) % 20) - 10;
                r = (unsigned char)(145 + glow);
                g = (unsigned char)(12 + glow / 4);
                b = (unsigned char)(18 + glow / 4);
            } else if (crack) {
                r = 76; g = 70; b = 86;
            } else if (shine) {
                r = 54; g = 48; b = 66;
            } else {
                int base = 22 + vary;
                if (base < 10) { base = 10; }
                if (base > 38) { base = 38; }
                r = (unsigned char)base;
                g = (unsigned char)(base - 2);
                b = (unsigned char)(base + 10);
            }

            int idx = (y * W + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_brick(Texture *t) {
    int brick_w = 16;
    int brick_h = 8;
    int mortar = 2;

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
                g = (unsigned char)(90 + vary / 2);
                b = (unsigned char)(70 + vary / 2);
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_stone(Texture *t) {
    int brick_w = 20;
    int brick_h = 14;
    int mortar = 2;

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
                r = 105; g = 105; b = 110;
            } else {
                int vary = ((x * 3 + y * 7) % 24) - 12;
                r = (unsigned char)(155 + vary);
                g = (unsigned char)(155 + vary);
                b = (unsigned char)(163 + vary);
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_sandstone(Texture *t) {
    int band_h = 8;
    int mortar = 1;

    for (int y = 0; y < t->height; y++) {
        int in_mortar = (y % band_h) < mortar;

        for (int x = 0; x < t->width; x++) {
            unsigned char r, g, b;
            if (in_mortar) {
                r = 140; g = 108; b = 62;
            } else {
                int vary = ((x * 2 + y * 5) % 20) - 10;
                r = (unsigned char)(198 + vary);
                g = (unsigned char)(163 + vary * 2 / 3);
                b = (unsigned char)(98 + vary / 3);
            }

            int idx = (y * t->width + x) * 3;
            t->pixels[idx] = r;
            t->pixels[idx + 1] = g;
            t->pixels[idx + 2] = b;
        }
    }
}

void texture_generate_weapon_kit_battle_rifle(Texture *t) {
    int W = t->width;
    int H = t->height;
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255;
        t->pixels[i + 1] = 0;
        t->pixels[i + 2] = 255;
    }

    for (int y = H * 25 / 64; y < H * 39 / 64; y++) {
        for (int x = W * 3 / 64; x < W * 19 / 64; x++) {
            int vary = ((x * 3 + y * 5) % 8) - 4;
            set_px(t, x, y, (unsigned char)(96 + vary), (unsigned char)(56 + vary), (unsigned char)(22 + vary));
        }
        set_px(t, W * 3 / 64, y, 34, 20, 9);
    }
    for (int y = H * 22 / 64; y < H * 40 / 64; y++) {
        for (int x = W * 18 / 64; x < W * 36 / 64; x++) {
            unsigned char v = (y < H * 25 / 64) ? 72 : 48;
            set_px(t, x, y, v, v, (unsigned char)(v + 5));
        }
    }
    for (int y = H * 24 / 64; y < H * 38 / 64; y++) {
        for (int x = W * 35 / 64; x < W * 49 / 64; x++) {
            int vary = ((x * 5 + y * 7) % 8) - 4;
            set_px(t, x, y, (unsigned char)(104 + vary), (unsigned char)(62 + vary), (unsigned char)(24 + vary));
        }
    }
    for (int y = H * 27 / 64; y < H * 32 / 64; y++) {
        for (int x = W * 48 / 64; x < W * 62 / 64; x++) {
            unsigned char v = (y == H * 27 / 64) ? 78 : 54;
            set_px(t, x, y, v, v, (unsigned char)(v + 5));
        }
    }
    for (int y = H * 25 / 64; y < H * 34 / 64; y++) {
        set_px(t, W * 61 / 64, y, 24, 24, 28);
    }
    for (int y = H * 39 / 64; y < H * 51 / 64; y++) {
        for (int x = W * 25 / 64; x < W * 34 / 64; x++) {
            int edge = x == W * 25 / 64 || x == W * 34 / 64 - 1 || y == H * 50 / 64;
            unsigned char v = edge ? 24 : 46;
            set_px(t, x, y, v, v, (unsigned char)(v + 5));
        }
    }
    for (int y = H * 27 / 64; y < H * 35 / 64; y++) {
        for (int x = W * 21 / 64; x < W * 29 / 64; x++) {
            set_px(t, x, y, 20, 20, 24);
        }
    }
    for (int x = W * 38 / 64; x < W * 47 / 64; x += W * 3 / 64) {
        for (int y = H * 26 / 64; y < H * 36 / 64; y++) {
            set_px(t, x, y, 62, 34, 13);
        }
    }
    for (int y = H * 18 / 64; y < H * 23 / 64; y++) {
        for (int x = W * 22 / 64; x < W * 26 / 64; x++) {
            set_px(t, x, y, 34, 34, 38);
        }
    }
    for (int y = H * 20 / 64; y < H * 25 / 64; y++) {
        for (int x = W * 52 / 64; x < W * 55 / 64; x++) {
            set_px(t, x, y, 34, 34, 38);
        }
    }
}

void texture_generate_weapon_kit_rifle_grenade(Texture *t) {
    texture_generate_weapon_kit_battle_rifle(t);
    int W = t->width;
    int H = t->height;
    for (int y = H * 20 / 64; y < H * 39 / 64; y++) {
        for (int x = W * 51 / 64; x < W * 61 / 64; x++) {
            int edge = x == W * 51 / 64 || x == W * 61 / 64 - 1 || y == H * 20 / 64 || y == H * 39 / 64 - 1;
            unsigned char value = edge ? 24 : 58;
            set_px(t, x, y, value, value, (unsigned char)(value + 5));
        }
    }
    for (int y = H * 22 / 64; y < H * 37 / 64; y++) {
        for (int x = W * 57 / 64; x < W * 63 / 64; x++) {
            int vary = ((x * 5 + y * 3) % 8) - 4;
            set_px(t, x, y, (unsigned char)(82 + vary), (unsigned char)(79 + vary), (unsigned char)(38 + vary));
        }
    }
}

void texture_generate_weapon_kit_ak47(Texture *t) {
    int W = t->width;
    int H = t->height;
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255;
        t->pixels[i + 1] = 0;
        t->pixels[i + 2] = 255;
    }

    /* wooden stock (left) */
    for (int y = H * 24 / 64; y < H * 38 / 64; y++) {
        for (int x = W * 4 / 64; x < W * 18 / 64; x++) {
            int vary = ((x * 3 + y * 5) % 6) - 3;
            set_px(t, x, y, (unsigned char)(90 + vary), (unsigned char)(52 + vary), (unsigned char)(18 + vary));
        }
    }
    /* stock end plate */
    for (int y = H * 24 / 64; y < H * 38 / 64; y++) {
        set_px(t, W * 4 / 64, y, 38, 22, 8);
        set_px(t, W * 5 / 64, y, 38, 22, 8);
    }

    /* receiver body */
    for (int y = H * 22 / 64; y < H * 36 / 64; y++) {
        for (int x = W * 16 / 64; x < W * 46 / 64; x++) {
            unsigned char v = (y < H * 24 / 64) ? 68 : 46;
            set_px(t, x, y, v, v, (unsigned char)(v + 4));
        }
    }
    /* charging handle nub on top */
    for (int y = H * 19 / 64; y < H * 23 / 64; y++) {
        for (int x = W * 20 / 64; x < W * 26 / 64; x++) {
            set_px(t, x, y, 58, 58, 62);
        }
    }
    /* ejection port */
    for (int y = H * 25 / 64; y < H * 33 / 64; y++) {
        for (int x = W * 18 / 64; x < W * 28 / 64; x++) {
            set_px(t, x, y, 24, 24, 28);
        }
    }

    /* barrel (pointing right) */
    for (int y = H * 26 / 64; y < H * 31 / 64; y++) {
        for (int x = W * 44 / 64; x < W * 62 / 64; x++) {
            set_px(t, x, y, 55, 55, 60);
        }
    }
    /* gas tube above barrel */
    for (int y = H * 23 / 64; y < H * 27 / 64; y++) {
        for (int x = W * 44 / 64; x < W * 58 / 64; x++) {
            set_px(t, x, y, 66, 66, 70);
        }
    }
    for (int y = H * 22 / 64; y < H * 34 / 64; y++) {
        for (int x = W * 43 / 64; x < W * 54 / 64; x++) {
            int grain = ((x * 5 + y * 7) % 7) - 3;
            set_px(t, x, y, (unsigned char)(104 + grain), (unsigned char)(61 + grain), (unsigned char)(22 + grain));
        }
    }
    /* muzzle end */
    for (int y = H * 25 / 64; y < H * 32 / 64; y++) {
        set_px(t, W * 61 / 64, y, 30, 30, 34);
        set_px(t, W * 62 / 64, y, 30, 30, 34);
    }

    /* pistol grip (wood) */
    for (int y = H * 35 / 64; y < H * 50 / 64; y++) {
        for (int x = W * 36 / 64; x < W * 46 / 64; x++) {
            int vary = ((x * 3 + y * 5) % 6) - 3;
            set_px(t, x, y, (unsigned char)(88 + vary), (unsigned char)(50 + vary), (unsigned char)(16 + vary));
        }
    }

    /* banana magazine — curves forward (rightward) as it descends */
    for (int y = H * 35 / 64; y < H * 43 / 64; y++) {
        for (int x = W * 20 / 64; x < W * 37 / 64; x++) {
            set_px(t, x, y, 48, 48, 52);
        }
        set_px(t, W * 22 / 64, y, 72, 72, 77);
    }
    for (int y = H * 43 / 64; y < H * 50 / 64; y++) {
        for (int x = W * 22 / 64; x < W * 39 / 64; x++) {
            set_px(t, x, y, 46, 46, 50);
        }
        set_px(t, W * 25 / 64, y, 68, 68, 73);
    }
    for (int y = H * 50 / 64; y < H * 56 / 64; y++) {
        for (int x = W * 26 / 64; x < W * 40 / 64; x++) {
            set_px(t, x, y, 42, 42, 46);
        }
    }
    /* magazine tip */
    for (int y = H * 56 / 64; y < H * 59 / 64; y++) {
        for (int x = W * 30 / 64; x < W * 39 / 64; x++) {
            set_px(t, x, y, 38, 38, 42);
        }
    }
}

void texture_generate_weapon_kit_dual(Texture *t) {
    int W = t->width;
    int H = t->height;
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255;
        t->pixels[i + 1] = 0;
        t->pixels[i + 2] = 255;
    }

    for (int gun = 0; gun < 2; gun++) {
        int x0 = gun == 0 ? W * 5 / 64 : W * 34 / 64;
        int x1 = gun == 0 ? W * 31 / 64 : W * 60 / 64;
        int y0 = gun == 0 ? H * 12 / 64 : H * 34 / 64;
        int dir = gun == 0 ? 1 : -1;

        for (int y = y0; y < y0 + H * 9 / 64; y++) {
            for (int x = x0; x < x1; x++) {
                int edge = x == x0 || x == x1 - 1 || y == y0 || y == y0 + H * 9 / 64 - 1;
                unsigned char v = edge ? 18 : (y < y0 + H * 3 / 64 ? 108 : 62);
                set_px(t, x, y, v, v, (unsigned char)(v + 6));
            }
        }

        int frame_x0 = dir > 0 ? x1 - W * 13 / 64 : x0;
        int frame_x1 = dir > 0 ? x1 : x0 + W * 13 / 64;
        for (int y = y0 + H * 9 / 64; y < y0 + H * 14 / 64; y++) {
            for (int x = frame_x0; x < frame_x1; x++) {
                set_px(t, x, y, 52, 52, 58);
            }
        }

        int grip_x0 = dir > 0 ? x1 - W * 8 / 64 : x0;
        int grip_x1 = grip_x0 + W * 8 / 64;
        for (int y = y0 + H * 13 / 64; y < y0 + H * 26 / 64; y++) {
            for (int x = grip_x0; x < grip_x1; x++) {
                int grain = ((x * 7 + y * 11) % 7) - 3;
                set_px(t, x, y, (unsigned char)(106 + grain), (unsigned char)(67 + grain), (unsigned char)(32 + grain));
            }
        }

        int muzzle_x = dir > 0 ? x0 : x1 - 1;
        for (int y = y0 + H * 2 / 64; y < y0 + H * 7 / 64; y++) {
            set_px(t, muzzle_x, y, 8, 8, 10);
        }
    }
}

void texture_generate_weapon_kit(Texture *t) {
    int W = t->width;
    int H = t->height;
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255;
        t->pixels[i + 1] = 0;
        t->pixels[i + 2] = 255;
    }

    /* side-profile pump-action shotgun, barrel pointing left */

    /* muzzle cap */
    for (int y = H * 24 / 64; y < H * 34 / 64; y++) {
        for (int x = W * 5 / 64; x < W * 9 / 64; x++) {
            set_px(t, x, y, 22, 22, 26);
        }
    }

    /* main barrel */
    for (int y = H * 26 / 64; y < H * 32 / 64; y++) {
        for (int x = W * 8 / 64; x < W * 36 / 64; x++) {
            unsigned char v = (y == H * 26 / 64 || y == H * 32 / 64 - 1) ? 30 : 58;
            unsigned char hi = (x < W * 12 / 64) ? 90 : v;
            set_px(t, x, y, hi, hi, (unsigned char)(hi + 8));
        }
    }

    /* under-barrel magazine tube */
    for (int y = H * 32 / 64; y < H * 37 / 64; y++) {
        for (int x = W * 8 / 64; x < W * 34 / 64; x++) {
            unsigned char v = (y == H * 32 / 64 || y == H * 37 / 64 - 1) ? 30 : 50;
            set_px(t, x, y, v, v, (unsigned char)(v + 6));
        }
    }

    /* pump forend (wood, wider band over barrel) */
    for (int y = H * 23 / 64; y < H * 40 / 64; y++) {
        for (int x = W * 16 / 64; x < W * 30 / 64; x++) {
            if (y >= H * 26 / 64 && y < H * 37 / 64) {
                int grain = ((x * 5 + y * 11) % 7) - 3;
                set_px(t, x, y,
                    (unsigned char)(92 + grain),
                    (unsigned char)(48 + grain),
                    (unsigned char)(14 + grain));
            } else {
                set_px(t, x, y, 62, 30, 8);
            }
        }
    }
    /* forend outline */
    for (int x = W * 16 / 64; x < W * 30 / 64; x++) {
        set_px(t, x, H * 23 / 64, 10, 10, 12);
        set_px(t, x, H * 40 / 64 - 1, 10, 10, 12);
    }
    for (int y = H * 23 / 64; y < H * 40 / 64; y++) {
        set_px(t, W * 16 / 64, y, 10, 10, 12);
        set_px(t, W * 30 / 64 - 1, y, 10, 10, 12);
    }

    /* receiver block */
    for (int y = H * 20 / 64; y < H * 44 / 64; y++) {
        for (int x = W * 30 / 64; x < W * 46 / 64; x++) {
            unsigned char v = (y < H * 22 / 64 || x > W * 44 / 64) ? 70 : 56;
            set_px(t, x, y, v, v, (unsigned char)(v + 6));
        }
    }
    /* ejection port */
    for (int y = H * 26 / 64; y < H * 36 / 64; y++) {
        for (int x = W * 32 / 64; x < W * 42 / 64; x++) {
            set_px(t, x, y, 14, 14, 16);
        }
    }

    for (int y = H * 24 / 64; y < H * 42 / 64; y++) {
        int inset = abs(y - H * 33 / 64) / 3;
        for (int x = W * 46 / 64 + inset; x < W * 62 / 64; x++) {
            int grain = ((x * 5 + y * 7) % 8) - 4;
            set_px(t, x, y, (unsigned char)(102 + grain), (unsigned char)(58 + grain), (unsigned char)(21 + grain));
        }
    }
    for (int y = H * 27 / 64; y < H * 39 / 64; y++) {
        set_px(t, W * 61 / 64, y, 42, 24, 9);
    }

}
