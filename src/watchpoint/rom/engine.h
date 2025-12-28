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

// USB data types
#define DATATYPE_TEXT        0x01
#define DATATYPE_RAWBINARY   0x02

// USB header macros
#define USBHEADER_GETTYPE(header) (((header) & 0xFF000000) >> 24)
#define USBHEADER_GETSIZE(header) (((header) & 0x00FFFFFF))

// Command codes
#define CMD_ECHO 0x03
#define CMD_DRAW_SQUARE 0x02

// PING/PONG messages
#define MSG_PING 0x50494E47
#define MSG_PONG 0x504F4E47

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
    // Input
    joypad_buttons_t btns;

    // FPS
    float fps;

    // USB
    int pkts_received;
    int last_datatype;
    int last_size;

    // Squares
    int square_idx;
    Square squares[MAX_SQUARES];

    // Debug
    char debug_str[0xFF];
} GameState;

void render();
void update();
void cmd_init();
void cmd_poll();

#endif
