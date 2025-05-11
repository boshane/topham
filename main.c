#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "topham.h"

struct game_t game;
SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *cursor;
TTF_Font *font;
struct button_t *action;
struct tracksel_t tracksel;
struct train_t train;
struct termios tios_new;
struct termios tios_prev;
struct winsize term_size;

SDL_Color buttontext = { 252, 220, 34 };

int
term_init()
{
	int retval;

	retval = tcgetattr(STDIN_FILENO, &tios_prev);
	if (retval == -1) {
		return -1;
	}

	retval = tcgetattr(STDIN_FILENO, &tios_new);
	if (retval == -1) {
		return -1;
	}

	tios_new.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
	tios_new.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | INPCK | IXON);
	tios_new.c_oflag &= ~(OPOST);
	tios_new.c_cc[VMIN] = 1;
	tios_new.c_cc[VTIME] = 0;

	retval = tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios_new);
	if (retval == -1) {
		return -1;
	}

	retval = ioctl(1, TIOCGWINSZ, &term_size);
	if (retval == -1) {
		return -1;
	}
}

void
init_assets()
{
	for (int i = 0; i < OBJ_END; i++) {
		SDL_Texture *obj_texture = IMG_LoadTexture(renderer,
		    objpath[i]);

		if (obj_texture == NULL) {
			obj_texture = IMG_LoadTexture(renderer,
			    "./resources/EMPTY.png");
		}
		SDL_QueryTexture(obj_texture, NULL, NULL,
		    &game.objdata[i].width, &game.objdata[i].height);
		game.objdata[i].desc = objdesc[i];
		game.objdata[i].texturep = obj_texture;
	}

	for (int i = 0; i < TRACK_END; i++) {
		SDL_Texture *track_texture = IMG_LoadTexture(renderer,
		    trackpath[i]);

		if (track_texture == NULL) {
			printf("Couldn't load image: %s\n", trackpath[i]);
			track_texture = IMG_LoadTexture(renderer,
			    "./resources/EMPTY.png");
		}
		game.trackdata[i].desc = trackdesc[i];
		game.trackdata[i].texturep = track_texture;
		SDL_QueryTexture(track_texture, NULL, NULL,
		    &game.trackdata[i].width, &game.trackdata[i].height);
	}

	for (int i = 0; i < TILE_END; i++) {
		SDL_Texture *tiledata_texture = IMG_LoadTexture(renderer,
		    tilepath[i]);

		if (tiledata_texture == NULL) {
			printf("Couldn't load image: %s\n", tilepath[i]);
			tiledata_texture = IMG_LoadTexture(renderer,
			    "./resources/EMPTY.png");
		}
		game.tiledata[i].tiletexture = tiledata_texture;
		game.tiledata[i].description = tiledesc[i];
	}

	for (int i = 0; i < 4; i++) {
		char buf[8];
		snprintf(buf, 7, "%d, %d", gradients[i].x, gradients[i].y);
		gradtextures[i] = get_text_texture(font, buf);
	}
}

void
initialize_train(struct train_t *train)
{
	train->traintex = IMG_LoadTexture(renderer, "./resources/train_engine.png");
	train->cars = 0;
	train->speed = 0;
	train->xy.x = STATION_START_X;
	train->xy.y = STATION_START_Y-3;
}

void
get_input(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT: {
			SDL_DestroyWindow(window);
			SDL_Quit();
			exit(0);
			break;
		}
		case SDL_MOUSEBUTTONDOWN: {
			switch (event.button.button) {
			case SDL_BUTTON_LEFT: {
				// TODO: cleanup
				if (game.hoveredbutton) {
					printf("%s\n",
					    game.hoveredbutton->string);
					game.hoveredbutton->fp(MAP_HEIGHT,
					    game.map);
					break;
				}
				if (tracksel.hovered != -1) {
					game.track = tracksel.hovered;
				}
				place_track(game.hoveredtile, game.track);
			}
			default:
				break;
			}
		}
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				game.camera.x = game.camera.x - CAMERA_SPEED;
				break;
			case SDLK_RIGHT:
				game.camera.x = game.camera.x + CAMERA_SPEED;
				break;
			case SDLK_UP:
				game.camera.y = game.camera.y - CAMERA_SPEED;
				break;
			case SDLK_DOWN:
				game.camera.y = game.camera.y + CAMERA_SPEED;
				break;
			default:
				break;
			}
		default:
			break;
		}
	}
}

void
draw_tile_surface()
{
	SDL_Rect dest;

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			dest.x = game.map[i][j].draw_from.x;
			dest.y = game.map[i][j].draw_from.y;

			if (game.map[i][j].object != OBJ_NONE) {
				dest.y = dest.y -
				    (game.objdata[game.map[i][j].object]
					    .height -
					TILE_HEIGHT);
				blit(game.objdata[game.map[i][j].object]
					 .texturep,
				    CAMX(dest.x), CAMY(dest.y));
			}
			if (game.map[i][j].track != TRACK_NONE) {
				dest.y = dest.y -
				    (game.trackdata[game.map[i][j].track]
					    .height -
					TILE_HEIGHT);
				blit(game.trackdata[game.map[i][j].track]
					 .texturep,
				    CAMX(dest.x), CAMY(dest.y));
			}
			if (&game.map[i][j] == game.hoveredtile)
				blit(cursor, CAMX(game.map[i][j].draw_from.x),
				    CAMY(game.map[i][j].draw_from.y));
		}
	}
}



void
mouse_tile_check(void)
{
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {
			double distance = distance_to_center(game.mousepos->x+game.camera.x,
			                                     game.mousepos->y+game.camera.y, 
											    game.map[i][j].center.x,
											    game.map[i][j].center.y);
			if (distance < 33) {
				game.hoveredtile = &game.map[i][j];
			}
		}
	}
}

int
mouse_ui_check(void)
{
	struct button_t *cur = action;

	while (cur) {
		if (SDL_PointInRect(game.mousepos, &cur->rect)) {
			game.hoveredbutton = cur;
			draw_square(cur->rect);

			return 1;
		}
		cur = cur->next;
	}
	game.hoveredbutton = NULL;

	if (SDL_PointInRect(game.mousepos, &tracksel.outerbg)) {
		for (int i = 0; i < TRACK_NONE; ++i) {
			if (SDL_PointInRect(game.mousepos,
				&tracksel.buttons[i]->rect)) {
				draw_diamond(tracksel.buttons[i]->rect);
				tracksel.hovered = i;
				return 1;
			}
		}
	}
	tracksel.hovered = -1;

	return 0;
}

void
draw_perlin_overlay(perlin_t *p)
{
	float step = MAP_WIDTH/p->size;

	for (int y = 0; y < p->size; y++) {
		for (int x = 0; x < p->size; x++) {
			int gindex = p->gradient_array[y][x];
			float xi = (float)x*step;
			float yi = (float)y*step;

//			printf("draw_perlin_overlay():\n\tsize: \t%d\n\tx,y:\t%d,%d\n\txi,yi\t%f,%f\n",p->size,x,y,xi,yi);
			int srcx = game.map[(int)yi][(int)xi].draw_from.x;
			int srcy = game.map[(int)yi][(int)xi].draw_from.y;

			SDL_Point src = { srcx, srcy };
			SDL_Point dst = { src.x+(gradients[gindex].x*100), src.y+(gradients[gindex].y*100)};

			v2vec *line = create_linevec(src, dst);
			
//			SDL_RenderSetScale(renderer, 3, 3);
			SDL_SetRenderDrawColor(renderer, 252, 250, 205, SDL_ALPHA_OPAQUE);
			SDL_RenderDrawPoints(renderer, line->points, line->len);
//			SDL_RenderSetScale(renderer, 1, 1);
			free(line);
			blit(gradtextures[gindex], CAMX(src.x), CAMY(src.y));
		}
	}
}

void
draw_debug()
{
	SDL_Texture *mousepos_texture;
	SDL_Texture *camerapos_texture;
	SDL_Texture *tilepos_texture;
	char *buf = calloc(DEBUG_TEXT_SIZE + 1, sizeof(char));

	snprintf(buf, DEBUG_TEXT_SIZE, "Mouse Position: %d, %d",
	    game.mousepos->x, game.mousepos->y);
	mousepos_texture = get_text_texture(font, buf);
	if (!mousepos_texture) {
		printf("mousepos_texture is null: %s\n", buf);
		return;
	}

	snprintf(buf, DEBUG_TEXT_SIZE, "Camera Position: %d, %d", game.camera.x,
	    game.camera.y);
	camerapos_texture = get_text_texture(font, buf);
	if (!camerapos_texture) {
		printf("camerapos_texture is null: %s\n", buf);
		return;
	}

	if (game.hoveredtile) {
		snprintf(buf, DEBUG_TEXT_SIZE, "Tile Mouseover: x: %d, y: %d",
		    game.hoveredtile?game.hoveredtile->x:0, game.hoveredtile?game.hoveredtile->y:0);
		tilepos_texture = get_text_texture(font, buf);
		if (!camerapos_texture) {
			printf("tilepos_texture is null: %s\n", buf);
			return;
		}
	}

//	draw_perlin_overlay(game.perlin);
	blit(mousepos_texture, CAMX(game.camera.x), CAMY(game.camera.y));
	blit(camerapos_texture, CAMX(game.camera.x), CAMY(game.camera.y + 20));
	blit(tilepos_texture, CAMX(game.camera.x), CAMY(game.camera.y + 40));
	SDL_DestroyTexture(mousepos_texture);
	SDL_DestroyTexture(camerapos_texture);
	SDL_DestroyTexture(tilepos_texture);
	free(buf);
}

void
cleanup()
{
	for (int i = 0; i < OBJ_END; i++) {
		SDL_DestroyTexture(game.objdata[i].texturep);
	}

	for (int i = 0; i < TRACK_END; i++) {
		SDL_DestroyTexture(game.trackdata[i].texturep);
	}

	for (int i = 0; i < TILE_END; i++) {
		SDL_DestroyTexture(game.tiledata[i].tiletexture);
	}
	perlin_free(game.perlin);
	SDL_DestroyRenderer(renderer);

	free(game.mousepos);
}

void
game_loop(void)
{
	game.running = true;

	while (game.running) {
		SDL_SetRenderDrawColor(renderer, 51, 153, 51, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_GetMouseState(&game.mousepos->x, &game.mousepos->y);
		print_perlin_averages(game.perlin, game.hoveredtile);
		draw_map();
		draw_tile_surface();
		ui_draw_actions(action);
		draw_tracksel(&tracksel);
		draw_debug();
		mouse_tile_check();
		mouse_ui_check();
		get_input();
		SDL_Delay(10);
		SDL_RenderPresent(renderer);
	}
}

void
quit_game()
{
	game.running = false;
}

int
main(void)
{
	game.mousepos = malloc(sizeof(SDL_Point));
	game.mousepos->x = 0;
	game.mousepos->y = 0;
	game.selected = NULL;
	game.hoveredtile = NULL;
	game.hoveredbutton = NULL;
	game.perlin = NULL;
	game.camera = (SDL_Rect) { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	game.track = 1;

	srand(time(NULL));
	generate_map(MAP_HEIGHT, game.map);

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Topham!", SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	cursor = IMG_LoadTexture(renderer, "./resources/tile_selector.png");

	if (TTF_Init() < 0) {
		printf("Couldn't initialize TTF: %s\n", SDL_GetError());
		return -1;
	}

	font = TTF_OpenFont("/usr/share/fonts/TTF/Hack-Bold.ttf", FONT_SIZE);
	if (!font) {
		printf("Couldn't initialize font: TTF_OpenFont: %s\n",
		    SDL_GetError());
		return -1;
	}

	if (term_init() != 0) {
		printf("terminal could not be initialized.\n");
		return -1;
	}
	init_assets();
	initialize_tracksel(&tracksel, font);
	initialize_train(&train);
	append_button(&action, "quit", NULL, quit_game, font);
	append_button(&action, "generate new map", NULL, generate_map, font);
	align_buttons(action);

	if (window == NULL) {
		SDL_Log("Could not create a renderer: %s", SDL_GetError());
		return -1;
	}
	SDL_RenderClear(renderer);

	game_loop();
	cleanup();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	CLEAR_CONSOLE();
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios_prev);
	return 0;
}
