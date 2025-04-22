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
//	draw_station(size, map);
	perlin_init(12, 16, &game.perlin);
	perlin(game.perlin);
	perlin_populate_map(size, map, game.perlin);
//	draw_river(size, map);
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

int
perlin_fill_gradients(perlin_t **per)
{
	int arrsize = (*per)->size;
	
	(*per)->gradient_array = malloc((arrsize+1) * sizeof(GradType*));

	if (!(*per)->gradient_array) {
		printf("perlin_fill_gradients(): malloc failed.\n");
		return -1;
	}
	for (int i = 0; i <= arrsize; i++) {
		(*per)->gradient_array[i] = malloc((arrsize+1) * (sizeof(GradType)));
		if (!(*per)->gradient_array[i]) {
			printf("perlin_fill_gradients(): malloc failed.\n");
			return -1;
		}
	}

	for (int i = 0; i <= arrsize; i++) {
		for (int j = 0; j <= arrsize; j++) {
			int index = rand() % 4;
			(*per)->gradient_array[i][j] = index;
		}
	}
	return 0;
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

int
perlin_init(int size, int density, perlin_t **per)
{
	if (*per) perlin_free(*per);

	(*per) = malloc(sizeof(perlin_t));

	if (!per) {
		printf("perlin_init(): malloc() failed\n");
		return -1;
	}

	(*per)->size = size;
	(*per)->density = density;

	perlin_fill_gradients(per);
	game.perlin = *per;

	return 0;
}

void
print_perlin_averages(perlin_t *per, maptile_t *hoveredtile)
{
	if (!hoveredtile) return;
	
	char mapgrid[11][256] = { '\0' };


	int startx = hoveredtile->x-5;
	int starty = hoveredtile->y-5;
	int mousex, mousey;

	startx<0?startx=0:0;
	starty<0?starty=0:0;

	for (int i = 0; i < 10; ++i) {
		snprintf(&mapgrid[i][32], 4, "   ");

		for (int j = 0; j < 10; ++j) {
			float cur = per->cbuf[starty+i][startx+j];
			
			do {
				if (j==0) {
					mapgrid[i][0] = ' ';
				} else mapgrid[i][(j*2)-1] = ' ';
				
				if (cur < -1.1) { mapgrid[i][j*2] = '_'; break; }
				if (cur < -.2) { mapgrid[i][j*2] = '~'; break; }
				if (cur < -.09) { mapgrid[i][j*2] = '-'; break; }
				if (cur < .08) { mapgrid[i][j*2] = '.'; break; }
				if (cur < .12) { mapgrid[i][j*2] = '^'; break; }
				if (cur < .6) { mapgrid[i][j*2] = '#'; break; }
				if (cur < .7) { mapgrid[i][j*2] = '*'; break; }
				mapgrid[i][j*2] = '.';
				break;

			} while(0);

			if (hoveredtile->x<5) {
				mousex = (5-(5-hoveredtile->x))*12+32;				
			}
			else if (hoveredtile->x>(MAP_WIDTH-5)) {
				mousex = ((j-(MAP_WIDTH-hoveredtile->x)+1)*12+32);
			}
			else mousex = 5*12+32;

			if (hoveredtile->y<5) {
				mousey = (5-(5-hoveredtile->y));				
			}
			else if (hoveredtile->y>(MAP_HEIGHT-5)) {
				mousey = ((i-(MAP_HEIGHT-hoveredtile->y)+1));
			}
			else mousey = 5;


			snprintf(&mapgrid[i][j*12+32], 10, "\t%.3f", cur);
		}
	}
	snprintf(&mapgrid[10][0], 48, "mousepos: %d, %d", mousex, mousey);
	printf("\x1b[1;1H");

	CLEAR_CONSOLE();

	for (int i = 0; i < 11; ++i) {
		for (int j = 0; j < 256; j++) {
			if (j==mousex && i==mousey) printf("\x1b[48;5;128m");			
			putchar(mapgrid[i][j]);
			if (j==mousex+6 && i==mousey) printf("\x1b[0m");
		}
		printf("\x1b[E");
	}
}

void
perlin_populate_map(int size, maptile_t map[][size], perlin_t *per)
{
	for (int i = 0; i < MAP_HEIGHT; ++i) {
		for (int j = 0; j < MAP_WIDTH; ++j) {
			float cur = per->cbuf[i][j];
			
			do {
				if (cur < -1.1) { map[i][j].tile = WATER; break; }
				if (cur < -.2) { map[i][j].object = OBJ_NONE; break; }
				if (cur < -.09) { map[i][j].object = OBJ_NONE; break; }
				if (cur < .08) { map[i][j].tile = GRASS_LONG; break; }
				if (cur < .12) { map[i][j].tile = ROCKS_LARGE; break; }
				if (cur < .6) { map[i][j].object = FOREST; break; }
				if (cur < .7) { map[i][j].object = FLORA; break; }
				map[i][j].tile = GRASS_SHORT;
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
	return t*t*t*(t*(t*6-15)+10);
}

void perlin(perlin_t *per)
{
	float topright, bottomright, bottomleft, topleft;
    float xf, yf;
    int xg, yg, xi, yi, gindex;
    enum MOD { TR, BR, BL, TL };
    const SDL_Point gmod[4] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
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
	int count = 0;

    for (int i = 0; i < per->size; ++i) {
	    for (int j = 0; j < per->size; ++j) {

	    	int xstep = 0;
	    	int ystep = 0;

			/* within each quadrant, iterate through the rows and columns */
	    	for (float si = div; ystep < per->pvalsize; si+=div, ystep++) {      
	    		for (float sj = div; xstep < per->pvalsize; sj+=div, xstep++) {
	    			
		    		for (int m = 0; m <= TL; ++m) {              
		    			/* starting with the top left corner, get the associated +/- x/y modifier */
				    	int xmod = gmod[m].x;           
				    	int ymod = gmod[m].y;
				

					    gindex = per->gradient_array[xmod+i][ymod+j];
				    	xg = gradients[gindex].x;
					    yg = gradients[gindex].y;
					    xi = (int)(i-i)+xmod;
					    yi = (int)(j-j)+ymod;
					    dp[m] = point_dot_product(xg, yg, sj-xi, si-yi);
	    			}
	    			float u = fade(si);
	    			float v = fade(sj);
				    float x1 = lerp(dp[TL], dp[TR], u);
				    float x2 = lerp(dp[BL], dp[BR], u);
				    float average = lerp(x1, x2, v);
//				    printf("perlin():\n\taverage: %f\n\tx1,x2:\t%f,%f\n\tsi, sj:\t%f,%f\n\ti,j:\t%d,%d\n\tcount: %d\n",average,x1,x2,si,sj,i,j,count);
				    per->cbuf[ystep+(i*per->pvalsize)][xstep+(j*per->pvalsize)] = average;
				    count++;
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
