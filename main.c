#include <math.h>
#include <stdio.h>
#include <time.h>
#include "topham.h"

struct game_t game;
SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *cursor;
TTF_Font *font;
struct button_t *action;
struct tracksel_t tracksel;

SDL_Color buttonbg = { 90, 24, 201 };
SDL_Color trackselbg = { 230, 230, 230 };
SDL_Color buttontext = { 252, 220, 34 };

void
blit(SDL_Texture *texture, int x, int y)
{
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;

	SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
	SDL_RenderCopy(renderer, texture, NULL, &dest);
}

int
initialize_font()
{
	font = TTF_OpenFont("/usr/share/fonts/TTF/Hack-Bold.ttf", FONT_SIZE);
	if (!font) {
		printf("Couldn't initialize font: TTF_OpenFont: %s\n",
		    SDL_GetError());
		return -1;
	}
	return 0;
}

struct button_t *
create_button(SDL_Texture *texture, const char *bstring)
{
	struct button_t *tmp = malloc(sizeof(struct button_t));

	if (!texture) {
		texture = get_text_texture(font, bstring);
	}
	tmp->label = texture;
	SDL_QueryTexture(tmp->label, NULL, NULL, &tmp->width, &tmp->height);
	tmp->string = bstring;
	return tmp;
}

void
initialize_tracksel(struct tracksel_t *ts)
{
	int ysum = BUTTON_BUFFER;

	for (int i = 0; i < TRACK_NONE; ++i) {
		ts->buttons[i] = create_button(game.trackdata[i].texturep,
		    trackdesc[i]);
		ts->buttons[i]->x = WINDOW_WIDTH - game.trackdata[i].width -
		    BUTTON_BUFFER;
		ts->buttons[i]->y = ysum;
		ts->buttons[i]->selected = i;
		ts->buttons[i]->rect = (SDL_Rect) { ts->buttons[i]->x,
			ts->buttons[i]->y, ts->buttons[i]->width,
			ts->buttons[i]->height };

		// don't buffer below the final track button
		if (i != TRACK_NONE - 1)
			ysum += ts->buttons[i]->height + BUTTON_BUFFER;
		else
			ysum += ts->buttons[i]->height;
	}
	ts->outerbg = (SDL_Rect) { ts->buttons[0]->x - 10,
		ts->buttons[0]->y - 10, ts->buttons[0]->width + 20, ysum + 20 };
	ts->innerbg = (SDL_Rect) { ts->buttons[0]->x, ts->buttons[0]->y,
		ts->buttons[0]->width, ysum };
}

void
draw_tracksel(struct tracksel_t *ts)
{
	SDL_SetRenderDrawColor(renderer, buttonbg.r, buttonbg.g, buttonbg.b,
	    SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &ts->outerbg);
	SDL_SetRenderDrawColor(renderer, trackselbg.r, trackselbg.g,
	    trackselbg.b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &ts->innerbg);

	for (int i = 0; i < TRACK_NONE; ++i) {
		if (game.track == i) {
			SDL_SetRenderDrawColor(renderer, 204, 255, 255,
			    SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(renderer, &ts->buttons[i]->rect);
		}
		struct button_t *cur = ts->buttons[i];
		blit(cur->label, cur->x, cur->y);
	}
}

void
append_button(struct button_t **root, char *text, SDL_Texture *btexture,
    void (*func)(int size, maptile_t[][size]))
{
	if (*root == NULL) {
		*root = create_button(btexture, text);
		(*root)->next = NULL;
		(*root)->fp = func;
		return;
	}

	struct button_t *prev = *root;
	struct button_t *cur = (*root)->next;

	while (cur) {
		prev = cur;
		cur = cur->next;
	}

	cur = create_button(btexture, text);
	cur->next = NULL;
	cur->fp = func;
	prev->next = cur;
}

void
align_buttons(struct button_t *root)
{
	if (root == NULL)
		return;

	int widthsum = 0;
	do {
		root->width = root->width + BUTTON_BUFFER;
		root->height = root->height + BUTTON_BUFFER;
		widthsum += root->width;
		root->x = WINDOW_WIDTH - widthsum - BUTTON_BUFFER;
		root->y = WINDOW_HEIGHT - root->height - BUTTON_BUFFER;
		root->rect = (SDL_Rect) { root->x, root->y, root->width,
			root->height };
		root = root->next;
		widthsum += BUTTON_BUFFER;
	} while (root);
}

void
ui_draw_actions(SDL_Renderer *renderer, struct button_t *button)
{
	if (!button)
		return;

	SDL_Rect bg = { button->x, button->y, button->width, button->height };
	SDL_SetRenderDrawColor(renderer, buttonbg.r, buttonbg.g, buttonbg.b,
	    SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &bg);
	blit(button->label, button->x + (BUTTON_BUFFER / 2),
	    button->y + (BUTTON_BUFFER / 2));

	ui_draw_actions(renderer, button->next);
}

void
init_assets()
{
	printf("Loading object data.\n");

	for (int i = 0; i < OBJ_END; i++) {
		SDL_Texture *obj_texture = IMG_LoadTexture(renderer,
		    objpath[i]);

		if (obj_texture == NULL) {
			printf("Couldn't load image: %s\n", objpath[i]);
			obj_texture = IMG_LoadTexture(renderer,
			    "./resources/EMPTY.png");
		}
		SDL_QueryTexture(obj_texture, NULL, NULL,
		    &game.objdata[i].width, &game.objdata[i].height);
		game.objdata[i].desc = objdesc[i];
		game.objdata[i].texturep = obj_texture;
		printf("%s: %d\n", game.objdata[i].desc,
		    game.objdata[i].colspan);
	}
	printf("Loading track data.\n");

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
	printf("Loading tile data.\n");

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
}

double
distance_to_center(int x, int y)
{
	int viewpointx = game.mousepos->x + game.camera.x;
	int viewpointy = game.mousepos->y + game.camera.y;

	float res = sqrt(pow(x - viewpointx, 2) + pow(y - viewpointy, 2) * 1.0);
	return res;
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
				} else if (game.hoveredtile->object ==
				    OBJ_NONE) {
					game.hoveredtile->track = game.track;
				}
				if (game.track == ERASER) {
					if (game.hoveredtile->track > 0) {
						game.hoveredtile->track =
						    TRACK_NONE;
					}
				} else
					printf(
					    "There is an object in the way.\n");
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
draw_map()
{
	SDL_Rect dest;

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			dest.x = game.map[i][j].draw_from.x;
			dest.y = game.map[i][j].draw_from.y;

			tiledata_t *cur = &game.tiledata[game.map[i][j].tile];

			blit(cur->tiletexture, CAMX(dest.x), CAMY(dest.y));
		}
	}
}

void
mouse_tile_check(void)
{
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {
			double distance = distance_to_center(
			    game.map[i][j].center.x, game.map[i][j].center.y);
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
		    game.hoveredtile->x, game.hoveredtile->y);
		tilepos_texture = get_text_texture(font, buf);
		if (!camerapos_texture) {
			printf("tilepos_texture is null: %s\n", buf);
			return;
		}
		SDL_DestroyTexture(tilepos_texture);
		blit(tilepos_texture, game.camera.x, game.camera.y + 40);
	}

	blit(mousepos_texture, CAMX(game.camera.x), CAMY(game.camera.y));
	blit(camerapos_texture, CAMX(game.camera.x), CAMY(game.camera.y + 20));
	SDL_DestroyTexture(mousepos_texture);
	SDL_DestroyTexture(camerapos_texture);
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
		draw_map();
		draw_tile_surface();
		ui_draw_actions(renderer, action);
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
	game.camera = (SDL_Rect) { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	game.track = 1;

	//	test = create_linevec((v2){.x = 0, .y = 0}, (v2){.x = 48, .y =
	// 40});
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
	initialize_font();
	init_assets();
	initialize_tracksel(&tracksel);
	append_button(&action, "quit", NULL, quit_game);
	append_button(&action, "generate new map", NULL, generate_map);
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
	return 0;
}
