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
}

int texture_recolor_uniform(Texture *dst, const Texture *src, unsigned char ur, unsigned char ug, unsigned char ub) {
    if (texture_create(dst, src->width, src->height) != 0) {
        return -1;
    }
    int n = src->width * src->height;
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
        } else {
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

void texture_generate_shotgun(Texture *t) {
    int W = t->width;
    int H = t->height;
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255; t->pixels[i + 1] = 0; t->pixels[i + 2] = 255;
    }
    /* Two barrel tubes pointing outward: run top-to-bottom, slightly inward taper */
    int lcx = W * 26 / 64;
    int rcx = W * 38 / 64;
    int br = 4;
    int barrel_top = H * 4 / 64;
    int barrel_bot = H * 38 / 64;
    for (int y = barrel_top; y < barrel_bot; y++) {
        int taper = (barrel_bot - y) * br / (barrel_bot - barrel_top + 1);
        int lr = br - taper / 6;
        if (lr < 2) { lr = 2; }
        for (int x = lcx - lr; x <= lcx + lr; x++) {
            unsigned char edge = (abs(x - lcx) >= lr) ? 40 : 58;
            set_px(t, x, y, edge, edge, edge + 6);
        }
        for (int x = rcx - lr; x <= rcx + lr; x++) {
            unsigned char edge = (abs(x - rcx) >= lr) ? 40 : 58;
            set_px(t, x, y, edge, edge, edge + 6);
        }
    }
    /* dark muzzle openings at top of each barrel */
    for (int y = barrel_top; y < barrel_top + 5; y++) {
        for (int dx = -3; dx <= 3; dx++) {
            set_px(t, lcx + dx, y, 18, 18, 20);
            set_px(t, rcx + dx, y, 18, 18, 20);
        }
    }
    /* receiver block joining both barrels */
    int rec_x0 = lcx - br - 3;
    int rec_x1 = rcx + br + 3;
    int rec_y0 = barrel_bot;
    int rec_y1 = H * 52 / 64;
    for (int y = rec_y0; y < rec_y1; y++) {
        for (int x = rec_x0; x <= rec_x1; x++) {
            int top_shade = (y == rec_y0) ? 70 : 52;
            set_px(t, x, y, (unsigned char)top_shade, (unsigned char)top_shade, (unsigned char)(top_shade + 5));
        }
    }
    /* grip: wood below receiver, narrower */
    int grip_x0 = W * 28 / 64;
    int grip_x1 = W * 36 / 64;
    for (int y = rec_y1; y < H - 2; y++) {
        for (int x = grip_x0; x <= grip_x1; x++) {
            int vary = ((x + y) % 6) - 3;
            set_px(t, x, y, (unsigned char)(88 + vary), (unsigned char)(52 + vary), (unsigned char)(18 + vary));
        }
    }
    /* trigger guard: small arc below receiver */
    int tg_y = rec_y1 + 3;
    int tg_cx = W / 2;
    for (int dx = -5; dx <= 5; dx++) {
        if (tg_cx + dx >= 0 && tg_cx + dx < W && tg_y < H) {
            set_px(t, tg_cx + dx, tg_y, 62, 62, 66);
        }
    }
}

void texture_generate_weapon_kit(Texture *t) {
    int W = t->width;
    int H = t->height;
    /* magenta background */
    for (int i = 0; i < W * H * 3; i += 3) {
        t->pixels[i] = 255; t->pixels[i + 1] = 0; t->pixels[i + 2] = 255;
    }

    /* side-profile pistol, barrel pointing left */

    /* barrel */
    int brl_x0 = W * 8 / 64;
    int brl_x1 = W * 30 / 64;
    int brl_y0 = H * 26 / 64;
    int brl_y1 = H * 32 / 64;
    for (int y = brl_y0; y < brl_y1; y++) {
        for (int x = brl_x0; x < brl_x1; x++) {
            set_px(t, x, y, 55, 55, 60);
        }
    }
    /* muzzle end — darker ring */
    for (int y = brl_y0 - 1; y < brl_y1 + 1; y++) {
        set_px(t, brl_x0, y, 30, 30, 34);
        set_px(t, brl_x0 + 1, y, 30, 30, 34);
    }

    /* slide / receiver block */
    int sx0 = W * 27 / 64;
    int sx1 = W * 55 / 64;
    int sy0 = H * 20 / 64;
    int sy1 = H * 38 / 64;
    for (int y = sy0; y < sy1; y++) {
        for (int x = sx0; x < sx1; x++) {
            unsigned char v = (y < sy0 + 2) ? 75 : 58;
            set_px(t, x, y, v, v, (unsigned char)(v + 5));
        }
    }
    /* ejection port cutout on slide */
    for (int y = sy0 + 4; y < sy1 - 5; y++) {
        for (int x = sx0 + 3; x < sx0 + 12; x++) {
            set_px(t, x, y, 22, 22, 26);
        }
    }

    /* frame below slide */
    int fy0 = sy1;
    int fy1 = H * 44 / 64;
    for (int y = fy0; y < fy1; y++) {
        for (int x = sx0 + 2; x < sx1; x++) {
            set_px(t, x, y, 50, 50, 55);
        }
    }

    /* trigger guard outline */
    int tgx0 = sx0 + 2;
    int tgx1 = W * 42 / 64;
    int tgy1 = H * 50 / 64;
    for (int x = tgx0; x <= tgx1; x++) {
        set_px(t, x, fy0, 44, 44, 48);
        set_px(t, x, tgy1, 44, 44, 48);
    }
    for (int y = fy0; y <= tgy1; y++) {
        set_px(t, tgx0, y, 44, 44, 48);
        set_px(t, tgx1, y, 44, 44, 48);
    }

    /* wood grip */
    int gx0 = W * 42 / 64;
    int gy1 = H * 58 / 64;
    for (int y = fy0; y < gy1; y++) {
        for (int x = gx0; x < sx1; x++) {
            int vary = ((x * 3 + y * 5) % 6) - 3;
            set_px(t, x, y, (unsigned char)(86 + vary), (unsigned char)(50 + vary), (unsigned char)(18 + vary));
        }
    }
}
