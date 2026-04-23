#include "core/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MAP_LINE 256

int map_load(Map *m, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "map_load: cannot open %s\n", path);
        return -1;
    }

    char lines[256][MAX_MAP_LINE];
    int height = 0;
    int width = 0;

    while (height < 256 && fgets(lines[height], MAX_MAP_LINE, f)) {
        int len = (int)strlen(lines[height]);
        while (len > 0 && (lines[height][len - 1] == '\n' || lines[height][len - 1] == '\r')) {
            lines[height][--len] = '\0';
        }
        if (len > width) {
            width = len;
        }
        height++;
    }
    fclose(f);

    if (width == 0 || height == 0) {
        fprintf(stderr, "map_load: empty map %s\n", path);
        return -1;
    }

    m->cells = calloc(width * height, sizeof(int));
    if (!m->cells) {
        return -1;
    }
    m->width = width;
    m->height = height;

    for (int y = 0; y < height; y++) {
        int len = (int)strlen(lines[y]);
        for (int x = 0; x < len; x++) {
            char c = lines[y][x];
            m->cells[y * width + x] = (c >= '1' && c <= '9') ? (c - '0') : 0;
        }
    }

    return 0;
}

void map_free(Map *m) {
    free(m->cells);
    m->cells = NULL;
    m->width = 0;
    m->height = 0;
}

int map_cell(const Map *m, int x, int y) {
    if (x < 0 || x >= m->width || y < 0 || y >= m->height) {
        return 1;
    }
    return m->cells[y * m->width + x];
}

int map_is_wall(const Map *m, int x, int y) {
    return map_cell(m, x, y) > 0;
}
