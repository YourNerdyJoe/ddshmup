#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void platform_draw_sprite(f32 dx, f32 dy, int sx, int sy, int w, int h);
void platform_draw_char(f32 dx, f32 dy, char c);

#ifdef __cplusplus
}
#endif
