#pragma once
#include "vec2.h"

//typedef struct SDL_Renderer SDL_Renderer;
//typedef struct SDL_Texture SDL_Texture;

void renderTextBounded(/*SDL_Renderer* renderer, SDL_Texture* font, int font_size,*/ ivec2 pos, int w, const char* text);
void renderText(/*SDL_Renderer* renderer, SDL_Texture* font, int font_size,*/ ivec2 pos, const char* text);
void renderTextFmtBounded(/*SDL_Renderer* renderer, SDL_Texture* font, int font_size,*/ ivec2 pos, int w, const char* fmt, ...);
void renderTextFmt(/*SDL_Renderer* renderer, SDL_Texture* font, int font_size,*/ ivec2 pos, const char* fmt, ...);
