#ifndef MAP_H
#define MAP_H

#define MAP_CELL_DOOR      10
#define MAP_CELL_DOOR_OPEN 11

typedef struct {
    int *cells;
    int width;
    int height;
} Map;

int  map_load(Map *m, const char *path);
void map_free(Map *m);
int  map_cell(const Map *m, int x, int y);
int  map_is_wall(const Map *m, int x, int y);
int  map_is_door(const Map *m, int x, int y);
void map_toggle_door(Map *m, int x, int y);

#endif
