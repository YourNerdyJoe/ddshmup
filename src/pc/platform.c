#include "../platform.h"
#include <SDL2/SDL.h>

static SDL_Renderer* renderer;
static SDL_Texture* atlas;
static SDL_Texture* font;

__declspec(dllexport) void platform_renderer_setup(SDL_Renderer* r, SDL_Texture* t, SDL_Texture* f) {
	renderer = r;
	atlas = t;
	font = f;
}

void platform_draw_sprite(f32 dx, f32 dy, int sx, int sy, int w, int h) {
	// switch(sprite_id) {
	// case 0: SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); break;
	// case 1: SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); break;
	// case 2: SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); break;
	// default: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
	// }

	// SDL_RenderFillRect(renderer, &(SDL_Rect){(int)x - 16, (int)y - 16, 32, 32});
	SDL_RenderCopy(renderer, atlas, &(SDL_Rect){sx, sy, w, h}, &(SDL_Rect){(int)dx, (int)dy, w, h});
}

void platform_draw_char(f32 dx, f32 dy, char c) {
	SDL_RenderCopy(renderer, font, &(SDL_Rect){c % 16 * 8, c / 16 * 8, 8, 8}, &(SDL_Rect){(int)dx, (int)dy, 8, 8});
}
