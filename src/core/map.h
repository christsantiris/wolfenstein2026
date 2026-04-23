#ifndef MAP_H
#define MAP_H

typedef struct {
    int *cells;
    int width;
    int height;
} Map;

int map_load(Map *m, const char *path);
void map_free(Map *m);
int map_cell(const Map *m, int x, int y);
int map_is_wall(const Map *m, int x, int y);

#endif
