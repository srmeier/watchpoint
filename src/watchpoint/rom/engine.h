#ifndef WATCHPOINT_ENGINE_H
#define WATCHPOINT_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include "debug.h"


#define MAX_SQUARES     512
#define CMD_PING        0x01
#define CMD_DRAW_SQUARE 0x02

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
    float fps;
    joypad_buttons_t btns;

    // Squares
    int square_idx;
    Square squares[MAX_SQUARES];

    // Debug
    int pkts_received;
    int last_size;
    char debug_str[0xFF];
} GameState;

void render();
void update();

char* process();
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length);

#endif
