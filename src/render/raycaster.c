#include "render/raycaster.h"
#include <math.h>

#define FOV_FACTOR  0.66f

#define CEILING_R   50
#define CEILING_G   50
#define CEILING_B   60

#define FLOOR_R     30
#define FLOOR_G     30
#define FLOOR_B     30

static void render_floor(SDL_Renderer *renderer, const Player *p, const Texture *floor_tex, int screen_w, int screen_h) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y = dir_x * FOV_FACTOR;
    float ray_left_x = dir_x - plane_x;
    float ray_left_y = dir_y - plane_y;
    float ray_right_x = dir_x + plane_x;
    float ray_right_y = dir_y + plane_y;
    int horizon = screen_h / 2;

    SDL_SetRenderDrawColor(renderer, FLOOR_R, FLOOR_G, FLOOR_B, 255);
    SDL_RenderDrawLine(renderer, 0, horizon, screen_w - 1, horizon);

    for (int y = horizon + 1; y < screen_h; y++) {
        float row_dist = (0.5f * screen_h) / (y - horizon);
        float floor_step_x = row_dist * (ray_right_x - ray_left_x) / screen_w;
        float floor_step_y = row_dist * (ray_right_y - ray_left_y) / screen_w;
        float floor_x = p->x + row_dist * ray_left_x;
        float floor_y = p->y + row_dist * ray_left_y;
        float shade = 1.0f / (1.0f + row_dist * 0.08f);
        if (shade < 0.28f) {
            shade = 0.28f;
        }

        for (int x = 0; x < screen_w; x++) {
            unsigned int colour = texture_sample(floor_tex, floor_x, floor_y);
            unsigned char r = (unsigned char)(((colour >> 16) & 0xFF) * shade);
            unsigned char g = (unsigned char)(((colour >> 8) & 0xFF) * shade);
            unsigned char b = (unsigned char)((colour & 0xFF) * shade);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
            floor_x += floor_step_x;
            floor_y += floor_step_y;
        }
    }
}

void raycaster_render(SDL_Renderer *renderer, const Map *m, const Player *p, const Texture *wall_tex, const Texture *floor_tex, const Texture *door_tex, const Texture *exit_tex, float *zbuf, int screen_w, int screen_h, float time) {
    float pulse = 0.5f + 0.5f * sinf(time * 3.0f);
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y = dir_x * FOV_FACTOR;

    render_floor(renderer, p, floor_tex, screen_w, screen_h);

    for (int x = 0; x < screen_w; x++) {
        float camera_x = 2.0f * x / screen_w - 1.0f;
        float ray_dx = dir_x + plane_x * camera_x;
        float ray_dy = dir_y + plane_y * camera_x;

        int map_x = (int)p->x;
        int map_y = (int)p->y;

        float delta_x = (ray_dx == 0.0f) ? 1e30f : fabsf(1.0f / ray_dx);
        float delta_y = (ray_dy == 0.0f) ? 1e30f : fabsf(1.0f / ray_dy);

        int step_x, step_y;
        float side_x, side_y;

        if (ray_dx < 0.0f) {
            step_x = -1;
            side_x = (p->x - map_x) * delta_x;
        } else {
            step_x = 1;
            side_x = (map_x + 1.0f - p->x) * delta_x;
        }

        if (ray_dy < 0.0f) {
            step_y = -1;
            side_y = (p->y - map_y) * delta_y;
        } else {
            step_y = 1;
            side_y = (map_y + 1.0f - p->y) * delta_y;
        }

        int side = 0;
        while (!map_is_wall(m, map_x, map_y)) {
            if (side_x < side_y) {
                side_x += delta_x;
                map_x += step_x;
                side = 0;
            } else {
                side_y += delta_y;
                map_y += step_y;
                side = 1;
            }
        }

        int cell_type = map_cell(m, map_x, map_y);

        /* For door/exit cells, determine the correct approach side from the map.
           A door flanked by walls in X has its slot in Y (correct side == 1).
           A door flanked by walls in Y has its slot in X (correct side == 0).
           If a ray hits from the wrong side, fall back to wall_tex so the
           door doesn't appear as a floating box when seen from the side. */
        int door_correct_side = -1;
        if (cell_type == MAP_CELL_DOOR || cell_type == MAP_CELL_EXIT) {
            int wall_left  = map_is_wall(m, map_x - 1, map_y);
            int wall_right = map_is_wall(m, map_x + 1, map_y);
            if (wall_left || wall_right) {
                door_correct_side = 1;
            } else {
                door_correct_side = 0;
            }
        }

        float perp_dist;
        float wall_x;
        if (side == 0) {
            perp_dist = (map_x - p->x + (1 - step_x) * 0.5f) / ray_dx;
            wall_x = p->y + perp_dist * ray_dy;
        } else {
            perp_dist = (map_y - p->y + (1 - step_y) * 0.5f) / ray_dy;
            wall_x = p->x + perp_dist * ray_dx;
        }
        if (perp_dist < 0.001f) {
            perp_dist = 0.001f;
        }
        zbuf[x] = perp_dist;
        wall_x -= floorf(wall_x);

        int wall_h = (int)(screen_h / perp_dist);
        int draw_start = (screen_h - wall_h) / 2;
        int draw_end = (screen_h + wall_h) / 2;
        if (draw_start < 0) {
            draw_start = 0;
        }
        if (draw_end > screen_h) {
            draw_end = screen_h;
        }

        SDL_SetRenderDrawColor(renderer, CEILING_R, CEILING_G, CEILING_B, 255);
        SDL_RenderDrawLine(renderer, x, 0, x, draw_start - 1);

        for (int y = draw_start; y < draw_end; y++) {
            unsigned char r, g, b;
            if (cell_type == MAP_CELL_VOID) {
                r = (unsigned char)(160 + 60 * pulse);
                g = (unsigned char)(120 + 60 * pulse);
                b = (unsigned char)(20 + 30 * pulse);
            } else {
                float tex_v = (y - (screen_h - wall_h) * 0.5f) / wall_h;
                const Texture *src;
                if (cell_type == MAP_CELL_DOOR && side == door_correct_side) {
                    src = door_tex;
                } else if (cell_type == MAP_CELL_EXIT && side == door_correct_side) {
                    src = exit_tex;
                } else {
                    src = wall_tex;
                }
                unsigned int colour = texture_sample(src, wall_x, tex_v);
                r = (colour >> 16) & 0xFF;
                g = (colour >> 8)  & 0xFF;
                b =  colour        & 0xFF;
                if (side == 1) {
                    r /= 2;
                    g /= 2;
                    b /= 2;
                }
            }
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}
