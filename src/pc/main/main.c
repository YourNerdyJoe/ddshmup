#include <SDL2/SDL.h>
#include "dll.h"
#include "../../game.h"

static const char* game_func_names[] = {
	"game_create",
	"game_update",
	"game_render",
	"platform_renderer_setup",
};

static struct {
	void* (*create)(size_t*);
	void (*update)(GameState*, VPadFrame, VPadFrame);
	void (*render)(const GameState*);
	void (*platform_renderer_setup)(SDL_Renderer*);
} game_api;

bool platform_input_poll(VPadFrame* pad) {
	pad->buttons = 0;
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
			case SDL_QUIT: return false;
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	pad->buttons |= keys[SDL_SCANCODE_LEFT] << B_LEFT;
	pad->buttons |= keys[SDL_SCANCODE_RIGHT] << B_RIGHT;
	pad->buttons |= keys[SDL_SCANCODE_UP] << B_UP;
	pad->buttons |= keys[SDL_SCANCODE_DOWN] << B_DOWN;
	pad->buttons |= keys[SDL_SCANCODE_Z] << B_A;
	pad->buttons |= keys[SDL_SCANCODE_X] << B_B;
	pad->buttons |= keys[SDL_SCANCODE_C] << B_C;
	return true;
}

int main(int argc, char* argv[]) {
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_Window* window = SDL_CreateWindow("ddshmup", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	if(!window) {
		SDL_Log("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!renderer) {
		SDL_Log("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	u64 dll_last_modified = getFileModifiedTime("libgame.dll");
	Dll* dll = dllLoad("libgame");
	dllGetFuncs(dll, (FnPtr*)&game_api, game_func_names, arrlen(game_func_names));
	game_api.platform_renderer_setup(renderer);

	size_t game_size;
	GameState* gs = game_api.create(&game_size);

	VPadFrame prev = {0};
	VPadFrame curr;
	while(platform_input_poll(&curr)) {
		game_api.update(gs, curr, prev);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		game_api.render(gs);

		SDL_RenderPresent(renderer);

		u64 new_timestamp;
		if(new_timestamp = getFileModifiedTime("libgame.dll"), dll_last_modified != new_timestamp) {
			dll_last_modified = new_timestamp;
			dllUnload(dll);
			dll = dllLoad("libgame");
			dllGetFuncs(dll, (FnPtr*)&game_api, game_func_names, arrlen(game_func_names));
			game_api.platform_renderer_setup(renderer);
		}

		prev = curr;

		//TODO: frame rate limiter, don't rely on vsync if releasing pc version
	}

	SDL_Quit();
	return 0;
}
