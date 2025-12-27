#ifndef WATCHPOINT_ENGINE_H
#define WATCHPOINT_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include "debug.h"


#define MAX_SQUARES 512

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int r;
    int g;
    int b;
    int a;
    int active;
} Square;

typedef struct {
    joypad_buttons_t btns;
    float fps;

    // Squares
    int square_idx;
    Square squares[MAX_SQUARES];
} GameState;

void render();
void update();
void cmd_init();

#endif
