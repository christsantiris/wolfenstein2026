#include "raycaster.h"
#include <math.h>

#define FOV_FACTOR   0.66f

#define CEILING_R    50
#define CEILING_G    50
#define CEILING_B    60

#define FLOOR_R      30
#define FLOOR_G      30
#define FLOOR_B      30

#define WALL_LIGHT_R 200
#define WALL_LIGHT_G 200
#define WALL_LIGHT_B 200

#define WALL_DARK_R  130
#define WALL_DARK_G  130
#define WALL_DARK_B  130

void raycaster_render(SDL_Renderer *renderer, const Map *m, const Player *p, int screen_w, int screen_h) {
    float dir_x = cosf(p->angle);
    float dir_y = sinf(p->angle);
    float plane_x = -dir_y * FOV_FACTOR;
    float plane_y = dir_x * FOV_FACTOR;

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

        float perp_dist;
        if (side == 0) {
            perp_dist = (map_x - p->x + (1 - step_x) * 0.5f) / ray_dx;
        } else {
            perp_dist = (map_y - p->y + (1 - step_y) * 0.5f) / ray_dy;
        }
        if (perp_dist < 0.001f) {
            perp_dist = 0.001f;
        }

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

        if (side == 0) {
            SDL_SetRenderDrawColor(renderer, WALL_LIGHT_R, WALL_LIGHT_G, WALL_LIGHT_B, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, WALL_DARK_R, WALL_DARK_G, WALL_DARK_B, 255);
        }
        SDL_RenderDrawLine(renderer, x, draw_start, x, draw_end);

        SDL_SetRenderDrawColor(renderer, FLOOR_R, FLOOR_G, FLOOR_B, 255);
        SDL_RenderDrawLine(renderer, x, draw_end + 1, x, screen_h - 1);
    }
}
