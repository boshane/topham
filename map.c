//
// Created by dan on 24/10/18.
//

#include <math.h>
#include "topham.h"

#define WINDOW_WIDTH  1600
#define WINDOW_HEIGHT 1200

extern struct game_t game;

void
generate_map(int size, maptile_t map[][size])
{
	fill_map_basetile(size, map);
//	populate_objects(size, map);
//	draw_river(size, map);
//	draw_river(size, map);
//	draw_station(size, map);
	perlin_init(4, 12, &game.perlin);
	perlin(game.perlin);
//	print_perlin_averages(game.perlin);
}

void
draw_station(int size, maptile_t map[][size])
{
	int x = STATION_START_X;
	int y = STATION_START_Y;

	map[x][y].object = STATION;
	map[x+1][y-6].track = STOPPER_BL;
	place_track(&map[x+1][y-5], TRBL);
	place_track(&map[x+1][y-4], TRBL);
	place_track(&map[x+1][y-3], TRBL);
	place_track(&map[x+1][y-2], TRBL);
	place_track(&map[x+1][y-1], TRBL);
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
}

void
perlin_fill_gradients(perlin_t **per)
{
	(*per)->gradient_array = malloc((*per)->size * sizeof(GradType*));
	for (int i = 0; i <= (*per)->size; i++) {
		(*per)->gradient_array[i] = malloc((*per)->size * (sizeof(GradType)));
	}

	for (int i = 0; i <= (*per)->size; i++) {
		for (int j = 0; j <= (*per)->size; j++) {
			int index = rand() % 4;
			(*per)->gradient_array[i][j] = index;
		}
	}
}

void
perlin_free(perlin_t *per)
{
	for (int i = 0; i < per->size; i++) {
		free(per->gradient_array[i]);
	}
	for (int i = 0; i < per->size*per->pvalsize; ++i) {
		free(per->cbuf[i]);
	}
	free(per);
}

void
perlin_init(int size, int density, perlin_t **per)
{
	if (*per) perlin_free(*per);

	(*per) = malloc(sizeof(perlin_t));
	(*per)->size = size;
	(*per)->density = density;

	perlin_fill_gradients(per);
	game.perlin = *per;
}

void
print_perlin_averages(perlin_t *per)
{
	for (int i = 0; i < per->size*per->pvalsize; ++i) {
		for (int j = 0; j < per->size*per->pvalsize; ++j) {
			float cur = per->cbuf[i][j];
			
			do {
				if (cur < -.5) { printf("_ "); break; }
				if (cur < -.2) { printf("~ "); break; }
				if (cur < -.09) { printf("- "); break; }
				if (cur < .2) { printf(". "); break; }
				if (cur < .8) { printf("^ "); break; }
				if (cur < .99) { printf("# "); break; }
				printf(". ");
				break;
			} while(0);
		}
		printf("\n");
	}
}

float lerp(float a, float b, float f)
{
	return a * (1.0 - f) + (b * f);
}

float point_dot_product(float xg, float yg, float xi, float yi)
{
	float dp = xg * xi + yg * yi;
	return dp;
}

/* from Ken Perlin's paper */
float fade(float t)
{
	return ((6*t - 15)*t+10)*t*t*t;
}

void perlin(perlin_t *per)
{
	float topright, bottomright, bottomleft, topleft;
    float xf, yf;
    int xg, yg, xi, yi, gindex;
    enum MOD { TR, BR, BL, TL };
    const SDL_Point modifiers[4] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
    float dp[TL+1];
    int pos = 0;
   	float div = .99/(float)per->density;                         
	per->pvalsize = (int)1/div;                                      

    int pvx = per->size*per->pvalsize;
    int pvy = per->size*per->pvalsize;
	per->cbuf = malloc(pvy*sizeof(float*));
	for (int i = 0; i < pvx; ++i) {
		per->cbuf[i] = malloc(pvx*sizeof(float)); 
	}

    for (int i = 0; i < per->size; ++i) {
	    for (int j = 0; j < per->size; ++j) {

	    	int xstep = 0;
	    	int ystep = 0;

	    	for (float si = div; si < 1; si+=div, ystep++) {      /* within each quadrant, iterate through the rows and columns */
	    		for (float sj = div; sj < 1; sj+=div, xstep++) {   
		    		for (int m = 0; m <= TL; ++m) {              /* iterate through the modifiers for each corner, and assign them */
				    	int xmod = modifiers[m].x;           
				    	int ymod = modifiers[m].y;
				
					    gindex = per->gradient_array[xmod+i][ymod+j];
				    	xg = gradients[gindex].x;
					    yg = gradients[gindex].y;
					    xi = (int)(i-i)+xmod;
					    yi = (int)(j-j)+ymod;
					    dp[m] = point_dot_product(xg, yg, sj-xi, si-yi);
	    			}
				    float x1 = lerp(dp[TL], dp[TR], si);
				    float x2 = lerp(dp[BL], dp[BR], si);
				    float average = lerp(x1, x2, sj);
				    per->cbuf[ystep+(i*per->pvalsize)][xstep+(j*per->pvalsize)] = average;
//					printf("%d, %d : ", xstep+(j*per->pvalsize), ystep+(i*per->pvalsize));
		    	}
		    	xstep = 0;
			}
	    }
    }
}

void
place_track(maptile_t *tile, Tracks selected)
{
	if (tile->object == OBJ_NONE) {
		if (selected == ERASER) {
			tile->track = TRACK_NONE;
		}	
		else tile->track = selected;
	}
	else printf("An object is in the way.\n");
}
