#include "topham.h"

extern struct game_t game;

double
distance(SDL_Point a, SDL_Point b)
{
	return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

v2vec *
create_linevec(SDL_Point veca, SDL_Point vecb)
{
	v2vec *line = malloc(sizeof(v2vec));
	line->len = distance(veca, vecb);
	line->len = line->len;
	line->points = malloc(sizeof(SDL_Point) * (line->len + 1));

	SDL_Point vecc = { .x = (vecb.x - veca.x), .y = (vecb.y - veca.y) };

	for (int i = 1; i < line->len; ++i) {
		line->points[i].x = veca.x + vecc.x * (float)i / line->len;
		line->points[i].y = veca.y + vecc.y * (float)i / line->len;
		line->points[i].x = CAMX(line->points[i].x);
		line->points[i].y = CAMY(line->points[i].y);
	}
	return line;
}

v2vec **
create_linevec_square(SDL_Point a, SDL_Point b, int scale)
{
	v2vec **square = malloc(sizeof(v2vec *) * 4);

	*square = create_linevec((SDL_Point) { a.x / scale, a.y / scale },
	    (SDL_Point) { b.x / scale, a.y / scale });
	*(square + 1) = create_linevec((SDL_Point) { b.x / scale, a.y / scale },
	    (SDL_Point) { b.x / scale, b.y / scale });
	*(square + 2) = create_linevec((SDL_Point) { b.x / scale, b.y / scale },
	    (SDL_Point) { a.x / scale, b.y / scale });
	*(square + 3) = create_linevec((SDL_Point) { a.x / scale, b.y / scale },
	    (SDL_Point) { a.x / scale, a.y / scale });

	return square;
}

v2vec **
create_linevec_diamond(SDL_Point a, SDL_Point b, int scale)
{
	v2vec **square = malloc(sizeof(v2vec *) * 4);

	int xh = (b.x - a.x) / 2;
	int yh = (b.y - a.y) / 2;

	*square = create_linevec((SDL_Point) { (a.x + xh) / scale,
				     a.y / scale },
	    (SDL_Point) { b.x / scale, (a.y + yh) / scale });
	*(square +
	    1) = create_linevec((SDL_Point) { b.x / scale, (a.y + yh) / scale },
	    (SDL_Point) { (b.x - xh) / scale, b.y / scale });
	*(square +
	    2) = create_linevec((SDL_Point) { (b.x - xh) / scale, b.y / scale },
	    (SDL_Point) { a.x / scale, (b.y - yh) / scale });
	*(square +
	    3) = create_linevec((SDL_Point) { a.x / scale, (b.y - yh) / scale },
	    (SDL_Point) { (a.x + xh) / scale, a.y / scale });

	return square;
}

double
distance_to_center(int vx, int vy, int x, int y)
{
	float res = sqrt(pow(x - vx, 2) + pow(y - vy, 2) * 1.0);
	return res;
}

