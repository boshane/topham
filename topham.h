#ifndef TOPHAM_H
#define TOPHAM_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define CAMX(i) (i - game.camera.x)
#define CAMY(i) (i - game.camera.y)

#define BUTTON_BUFFER 20

#define WINDOW_WIDTH  1600
#define WINDOW_HEIGHT 1200

#define FONT_SIZE     22

#define MAP_WIDTH	150
#define MAP_HEIGHT	150

#define MAX_RIVER_WIDTH 5

#define MODNUM		25

#define TILE_HEIGHT	60
#define TILE_WIDTH	120

#define DEBUG_TEXT_SIZE 48

#define CAMERA_SPEED	50

#define X_TRACKTYPES  \
	X(TLBL)       \
	X(TRBR)       \
	X(TRBL)       \
	X(TLBR)       \
	X(STOPPER_BL) \
	X(ERASER)     \
	X(TRACK_NONE)

#define X_OBJTYPES    \
	X(FLORA)      \
	X(FOREST)     \
	X(LOGS)       \
	X(BARN_LEFT)  \
	X(BARN_RIGHT) \
	X(STATION)    \
	X(OBJ_NONE)

#define X_TILETYPES     \
	X(GRASS_SHORT)  \
	X(ROAD)         \
	X(POND)         \
	X(GRASS_LONG)   \
	X(WATER)        \
	X(WATER_FROZEN) \
	X(ROCKS_SMALL)  \
	X(ROCKS_LARGE)  \
	X(TILE_NONE)

#define X(name) name,
typedef enum { X_TRACKTYPES TRACK_END } Tracks;
#undef X

#define X(name) "./resources/track_" #name ".png",
static const char *trackpath[] = { X_TRACKTYPES };
#undef X

#define X(name) name,
typedef enum { X_OBJTYPES OBJ_END } Objects;
#undef X

#define X(name) "./resources/obj_" #name ".png",
static const char *objpath[] = { X_OBJTYPES };
#undef X

#define X(name) name,
typedef enum { X_TILETYPES TILE_END } Tiles;
#undef X

#define X(name) "./resources/tile_" #name ".png",
static const char *tilepath[] = { X_TILETYPES };

#undef X
typedef struct {
	char *desc;
	int width, height;
	int colspan;
	SDL_Texture *texturep;
} objdata_t;

typedef struct {
	int x, y;
} Point;

static const Point modifiers[MODNUM] = { { -1, -1 }, { -1, 0 }, { -1, 1 },
	{ 0, -1 }, { 0, 0 }, { 0, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 },
	{ -2, -2 }, { -2, -1 }, { -2, 0 }, { -2, 1 }, { -2, 2 }, { -1, -2 },
	{ -1, 2 }, { 0, -2 }, { 0, 2 }, { 1, -2 }, { 1, 2 }, { 2, -2 },
	{ 2, -1 }, { 2, 0 }, { 2, 1 }, { 2, 2 } };

static char *trackdesc[TRACK_END] = { "Curve from top left to bottom left",
	"Curve from top right to bottom right",
	"Straight from top right to bottom left",
	"Straight from top left to bottom right", "Bottom left stopper",
	"Track eraser", "NONE" };

static char *objdesc[OBJ_END] = { "Light flora", "Forest", "Logs", "Barn",
	"Barn", "Station", "NONE" };

static char *tiledesc[TILE_END] = {
	"Short grass",
	"Road",
	"Fresh water",
	"Long grass",
	"Water",
	"Frozen water",
	"Small rocks",
	"Large rocks",
};

typedef struct {
	int x, y;
	struct SDL_Point center;
	struct SDL_Point draw_from;
	Tiles tile;
	Objects object;
	Tracks track;
} maptile_t;

typedef struct {
	char *desc;
	int width, height;
	SDL_Texture *texturep;
} trackdata_t;

typedef struct {
	char *name;
	char *description;
	int width, height;
	SDL_Texture *tiletexture;
	SDL_Texture *tiledesc;
} tiledata_t;

typedef struct {
	SDL_Point *points;
	float len;
} v2vec;

struct game_t {
	objdata_t objdata[OBJ_END];
	trackdata_t trackdata[TRACK_END];
	tiledata_t tiledata[TILE_END];
	maptile_t *hoveredtile;
	Tracks track;
	struct button_t *hoveredbutton;
	maptile_t *selected;
	struct SDL_Point *mousepos;
	maptile_t map[MAP_HEIGHT][MAP_WIDTH];
	SDL_Rect camera;
	bool running;
};

struct button_t {
	int x, y;
	int width, height;
	SDL_Texture *label;
	const char *string;
	struct button_t *next;
	Tracks selected;
	SDL_Rect rect;
	void (*fp)(int size, maptile_t map[][size]);
};

struct tracksel_t {
	SDL_Rect outerbg;
	SDL_Rect innerbg;
	Tracks hovered;
	struct button_t *buttons[TRACK_NONE];
};

void *draw_square(SDL_Rect r);
void *draw_diamond(SDL_Rect r);
SDL_Texture *get_text_texture(TTF_Font *font, const char *text);

void append_button(struct button_t **root, char *text, SDL_Texture *btexture,
    void (*func)());
void append_tracksel(struct button_t **root, SDL_Texture *btexture,
    void (*func)());

void generate_map(int size, maptile_t map[][size]);
void fill_map_basetile(int size, maptile_t map[][size]);
void draw_river(int size, maptile_t map[][size]);
void clear_tracks(int size, maptile_t map[][size]);
void populate_objects(int size, maptile_t map[][size]);

double distance(SDL_Point a, SDL_Point b);
v2vec *create_linevec(SDL_Point veca, SDL_Point vecb);
v2vec **create_linevec_square(SDL_Point a, SDL_Point b, int scale);
v2vec **create_linevec_diamond(SDL_Point a, SDL_Point b, int scale);

#endif
