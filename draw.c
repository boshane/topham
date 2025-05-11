#include "topham.h"

extern SDL_Renderer *renderer;
extern struct game_t game;

SDL_Color buttonbg = { 90, 24, 201 };
SDL_Color trackselbg = { 230, 230, 230 };

int
blit(SDL_Texture *texture, int x, int y)
{
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;

	/* get the width and height of the texture */
	if (SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h) != 0) {
//		printf("blit() x: %d, y: %d -- SDL_QueryTexture() failed; %s\n", x, y, SDL_GetError());
		return -1;
	}

	/* draw it on the renderer */
	if (SDL_RenderCopy(renderer, texture, NULL, &dest) != 0) {
		printf("blit(): SDL_RenderCopy() failed\n");
		return -1;
	}
	return 0;
}

SDL_Texture *
get_text_texture(TTF_Font *font, const char *text)
{
	SDL_Surface *surface;
	SDL_Texture *texture;

	surface = TTF_RenderText_Solid(font, text,
	    (SDL_Color) { 252, 220, 32 });
	if (surface == NULL) {
		printf("get_text_texture(): %s\n", SDL_GetError());
		return NULL;
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == NULL) {
		printf("get_text_texture(): %s\n", SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	return texture;
}

void *
draw_square(SDL_Rect r)
{
	v2vec **s = create_linevec_square((SDL_Point) { r.x, r.y },
	    (SDL_Point) { r.x + r.w, r.y + r.h }, 3);

	SDL_SetRenderDrawColor(renderer, 252, 50, 35, SDL_ALPHA_OPAQUE);
	SDL_RenderSetScale(renderer, 3, 3);
	SDL_RenderDrawPoints(renderer, s[0]->points, s[0]->len);
	SDL_RenderDrawPoints(renderer, s[1]->points, s[1]->len);
	SDL_RenderDrawPoints(renderer, s[2]->points, s[2]->len);
	SDL_RenderDrawPoints(renderer, s[3]->points, s[3]->len);
	SDL_RenderSetScale(renderer, 1, 1);
}

void *
draw_diamond(SDL_Rect r)
{
	v2vec **d = create_linevec_diamond((SDL_Point) { r.x, r.y },
	    (SDL_Point) { r.x + r.w, r.y + r.h }, 3);

	SDL_SetRenderDrawColor(renderer, 252, 50, 35, SDL_ALPHA_OPAQUE);
	SDL_RenderSetScale(renderer, 3, 3);
	SDL_RenderDrawPoints(renderer, d[0]->points, d[0]->len);
	SDL_RenderDrawPoints(renderer, d[1]->points, d[1]->len);
	SDL_RenderDrawPoints(renderer, d[2]->points, d[2]->len);
	SDL_RenderDrawPoints(renderer, d[3]->points, d[3]->len);
	SDL_RenderSetScale(renderer, 1, 1);
}

void
initialize_tracksel(struct tracksel_t *ts, TTF_Font *font)
{
	int ysum = BUTTON_BUFFER;

	for (int i = 0; i < TRACK_NONE; ++i) {
		ts->buttons[i] = create_button(game.trackdata[i].texturep,
		    trackdesc[i], font);
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

struct button_t *
create_button(SDL_Texture *texture, const char *bstring, TTF_Font *font)
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
draw_train(struct train_t *train)
{
	SDL_Rect dest;
	
	int destx = game.map[train->xy.x][train->xy.y].draw_from.x;
	int desty = game.map[train->xy.x][train->xy.y].draw_from.y;
	dest.x = destx;
	dest.y = desty-28;

	SDL_QueryTexture(train->traintex, NULL, NULL, &destx, &desty);
	SDL_RenderCopyEx(renderer, train->traintex, NULL, &dest, 335, NULL, SDL_FLIP_NONE);
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
		int res = blit(cur->label, cur->x, cur->y);
		if (res != 0) exit(0);
	}
}

void
ui_draw_actions(struct button_t *button)
{
	if (!button)
		return;

	SDL_Rect bg = { button->x, button->y, button->width, button->height };
	SDL_SetRenderDrawColor(renderer, buttonbg.r, buttonbg.g, buttonbg.b,
	    SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &bg);
	blit(button->label, button->x + (BUTTON_BUFFER / 2),
	    button->y + (BUTTON_BUFFER / 2));

	ui_draw_actions(button->next);
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
append_button(struct button_t **root, char *text, SDL_Texture *btexture,
    void (*func)(int size, maptile_t[][size]), TTF_Font *font)
{
	if (*root == NULL) {
		*root = create_button(btexture, text, font);
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

	cur = create_button(btexture, text, font);
	cur->next = NULL;
	cur->fp = func;
	prev->next = cur;
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

			int res = blit(cur->tiletexture, CAMX(dest.x), CAMY(dest.y));
			if (res != 0) exit(0);
		}
	}
}
