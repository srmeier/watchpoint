#include "commands.h"
#include <string.h>

/* Array of squares */
static Square squares[MAX_SQUARES];
static int square_count = 0;

void commands_init(void) {
    int i;

    /* Initialize all squares as inactive */
    for (i = 0; i < MAX_SQUARES; i++) {
        squares[i].active = 0;
    }
    square_count = 0;
}

void commands_process(u8* data, u32 size) {
    u8 cmd;
    s32 x, y, z, sq_size, rotation;
    u8 r, g, b, a;
    int i;

    if (size < 1) {
        return;
    }

    cmd = data[0];

    switch (cmd) {
        case CMD_ADD_SQUARE:
            /* Packet format:
             * [CMD:1] [x:4] [y:4] [z:4] [size:4] [rotation:4] [r:1] [g:1] [b:1] [a:1]
             * Total: 25 bytes (may be padded to 26 for alignment)
             */
            if (size < 25 || size > 26) {
                return;  /* Invalid packet */
            }

            /* Extract values (big-endian) */
            x = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
            y = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];
            z = (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12];
            sq_size = (data[13] << 24) | (data[14] << 16) | (data[15] << 8) | data[16];
            rotation = (data[17] << 24) | (data[18] << 16) | (data[19] << 8) | data[20];
            r = data[21];
            g = data[22];
            b = data[23];
            a = data[24];

            /* Find inactive slot or reuse oldest */
            for (i = 0; i < MAX_SQUARES; i++) {
                if (!squares[i].active) {
                    break;
                }
            }

            /* If all slots full, use slot 0 (oldest) */
            if (i >= MAX_SQUARES) {
                i = 0;
            }

            /* Store square data */
            squares[i].x = x;
            squares[i].y = y;
            squares[i].z = z;
            squares[i].size = sq_size;
            squares[i].rotation = rotation;
            squares[i].r = r;
            squares[i].g = g;
            squares[i].b = b;
            squares[i].a = a;
            squares[i].active = 1;

            /* Update count */
            square_count = 0;
            for (i = 0; i < MAX_SQUARES; i++) {
                if (squares[i].active) {
                    square_count++;
                }
            }

            break;

        default:
            /* Unknown command - ignore */
            break;
    }
}

Square* commands_get_squares(void) {
    return squares;
}

int commands_get_square_count(void) {
    return square_count;
}
