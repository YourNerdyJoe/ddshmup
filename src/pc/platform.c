#include "../platform.h"
#include <SDL2/SDL.h>

static SDL_Renderer* renderer;

__declspec(dllexport) void platform_renderer_setup(SDL_Renderer* r) {
	renderer = r;
}

void platform_draw_sprite(u32 sprite_id, f32 x, f32 y) {
	switch(sprite_id) {
	case 0: SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); break;
	case 1: SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); break;
	case 2: SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); break;
	default: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
	}

	SDL_RenderFillRect(renderer, &(SDL_Rect){(int)x - 16, (int)y - 16, 32, 32});
}
