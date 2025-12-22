#ifndef COMMANDS_H
#define COMMANDS_H

#include <nusys.h>

/* Command IDs */
#define CMD_ADD_SQUARE 0x01
#define CMD_ECHO 0x02
#define CMD_ADD 0x03

/* Square structure */
typedef struct {
    s32 x;
    s32 y;
    s32 z;
    s32 size;
    s32 rotation;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
    u8 active;
} Square;

/* Maximum number of squares */
#define MAX_SQUARES 32

/* Initialize command system */
void commands_init(void);

/* Process incoming USB command */
void commands_process(u8* data, u32 size);

/* Get the array of squares for rendering */
Square* commands_get_squares(void);

/* Get number of active squares */
int commands_get_square_count(void);

#endif
