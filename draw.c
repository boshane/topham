#include "topham.h"

extern SDL_Renderer *renderer;

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
