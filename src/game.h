#pragma once
#include "types.h"

enum {
	B_LEFT,
	B_RIGHT,
	B_UP,
	B_DOWN,
	B_A,
	B_B,
	B_C,
};

typedef struct GameState GameState;

typedef struct VPadFrame {
	u16 buttons;
} VPadFrame;
