//
// Created by dan on 24/10/18.
//

#include <math.h>
#include "topham.h"

#define WINDOW_WIDTH  1600
#define WINDOW_HEIGHT 1200

void
generate_map(int size, maptile_t map[][size])
{
	fill_map_basetile(size, map);
	populate_objects(size, map);
	draw_river(size, map);
	draw_river(size, map);
}

void
fill_map_basetile(int SIZE, maptile_t map[][SIZE])
{
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			SDL_Point center;
			center.x = ((WINDOW_WIDTH / 2) - (TILE_WIDTH / 2) * j) +
			    (TILE_HEIGHT * i);
			center.y = (TILE_HEIGHT / 2) +
			    (((TILE_HEIGHT / 2) * j) + (TILE_HEIGHT / 2) * i);
			map[i][j].draw_from.x = center.x - (TILE_WIDTH / 2);
			map[i][j].draw_from.y = center.y - (TILE_HEIGHT / 2);
			map[i][j].center = center;
			map[i][j].x = j;
			map[i][j].y = i;
			map[i][j].tile = GRASS_SHORT;
			map[i][j].track = TRACK_NONE;
			map[i][j].object = OBJ_NONE;
		}
	}
}

void
clear_tracks(int size, maptile_t map[][size])
{
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[i][j].track = TRACK_NONE;
		}
	}
}

void
draw_river(int size, maptile_t map[][size])
{
	float x, xmod;
	x = -3.14159;

	int startx = rand() % size / 2;
	startx += 10;
	int endx = rand() % size / 2;
	endx += 10;
	printf("%d, %d\n", startx, endx);

	v2vec *river = create_linevec((SDL_Point) { endx, 0 },
	    (SDL_Point) { startx, size });

	int width = rand() % MAX_RIVER_WIDTH;

	for (int i = 1; i < river->len; ++i) {
		SDL_Point p = river->points[i];
		xmod = sin(x);
		x += .1;
		xmod = xmod * 10;

		map[p.y][p.x + (int)xmod].tile = WATER;
		map[p.y][p.x + (int)xmod].object = OBJ_NONE;

		for (int i = 0; i < width; ++i) {
			map[p.y][p.x + (int)xmod + i].tile = WATER;
			map[p.y][p.x + (int)xmod + i].object = OBJ_NONE;
		}
	}
	free(river);
}

void
populate_objects(int size, maptile_t map[][size])
{
	for (int i = 0; i < MAP_HEIGHT * .5; i++) {
		int x = rand() % MAP_HEIGHT;
		int y = rand() % MAP_WIDTH;
		int density = 8;

		for (int j = 0; j < MODNUM; ++j) {
			int dcheck = rand() % 10;
			int mx = x + modifiers[j].x;
			int my = y + modifiers[j].y;

			if ((mx >= 0 && my >= 0) &&
			    (mx < MAP_WIDTH && my < MAP_HEIGHT)) {
				if (density >= dcheck) {
					int fcheck = rand() % 10;
					map[my][mx].object = fcheck > 2 ?
					    FOREST :
					    FLORA;
				}
			}
			if (j == 8) {
				density = density / 3;
			}
		}
	}
	map[2][8].object = STATION;
}
