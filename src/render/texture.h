#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
    unsigned char *pixels;
    int width;
    int height;
} Texture;

int texture_create(Texture *t, int width, int height);
int texture_load_ppm(Texture *t, const char *path);
void texture_free(Texture *t);
unsigned int texture_sample(const Texture *t, float u, float v);
void texture_generate_brick(Texture *t);
void texture_generate_stone(Texture *t);
void texture_generate_sandstone(Texture *t);
void texture_generate_blue_brick(Texture *t);
void texture_generate_wood(Texture *t);
void texture_generate_moss_stone(Texture *t);
void texture_generate_door(Texture *t);
void texture_generate_exit_door(Texture *t);
void texture_generate_guard_dir(Texture *t, int dir);
void texture_derive_guard_dirs(Texture guard_tex[8]);
int  texture_recolor_uniform(Texture *dst, const Texture *src, unsigned char ur, unsigned char ug, unsigned char ub);
void texture_generate_ammo_pickup(Texture *t);
void texture_generate_health_pickup(Texture *t);
void texture_generate_weapon_kit(Texture *t);
void texture_generate_weapon_kit_ak47(Texture *t);

#endif
