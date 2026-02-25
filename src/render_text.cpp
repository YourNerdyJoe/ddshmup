#include "render_text.h"
//#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>
//#include "common/rect.h"
#include "platform.h"

constexpr int font_size = 8;

void renderTextBounded(ivec2 pos, int w, const char* text) {
	int curr_x = pos.x;
	int curr_y = pos.y;
	while(*text) {
		if(*text == '\n') {
			curr_x = pos.x;
			curr_y += font_size;
			text++;
		}
		else if(w > 0 && *text == ' ') {
			while(*text && *text == ' ') {
				curr_x += font_size;
				text++;
			}
			const char* word = text;
			int word_width = 0;
			while(*word && *word != ' ' && *word != '\n') {
				word_width += font_size;
				word++;
			}
			if(word_width + curr_x - pos.x > w) {
				curr_x = pos.x;
				curr_y += font_size;
			}
		}
		else {
			//Rect ch_rect = {(*text%16)*8, (*text/16)*8, 8, 8};
			//Rect dst_rect = {curr_x, curr_y, font_size, font_size};
			//SDL_RenderCopy(renderer, font, (SDL_Rect*)&ch_rect, (SDL_Rect*)&dst_rect);
			platform_draw_char((f32)curr_x, (f32)curr_y, *text);
			curr_x += font_size;
			text++;
		}
	}
}

void renderText(ivec2 pos, const char* text) {
	renderTextBounded(pos, 0, text);
}

void renderTextFmtBounded(ivec2 pos, int w, const char* fmt, ...) {
	va_list ap;
	char buffer[512];
	va_start(ap, fmt);
	vsnprintf(buffer, 512, fmt, ap);
	renderTextBounded(pos, w, buffer);
	va_end(ap);
}

void renderTextFmt(ivec2 pos, const char* fmt, ...) {
	va_list ap;
	char buffer[512];
	va_start(ap, fmt);
	vsnprintf(buffer, 512, fmt, ap);
	renderText(pos, buffer);
	va_end(ap);
}
