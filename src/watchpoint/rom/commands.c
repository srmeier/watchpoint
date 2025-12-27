#include "engine.h"


extern GameState state;

char* cmd_ping()
{
    return "pong";
}

char* cmd_draw_square()
{
    int size;
    char x[4], y[4], w[4], h[4] = {0};
    char r[4], g[4], b[4], a[4] = {0};

    // Add square to state
    if (state.square_idx >= MAX_SQUARES) {
        state.square_idx = 0;
    }

    Square* sq = &state.squares[state.square_idx++];

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid x cordinate";
    debug_parsecommand(x);
    sq->x = atoi(x);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid y cordinate";
    debug_parsecommand(y);
    sq->y = atoi(y);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid width";
    debug_parsecommand(w);
    sq->w = atoi(w);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid height";
    debug_parsecommand(h);
    sq->h = atoi(h);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid red value";
    debug_parsecommand(r);
    sq->r = atoi(r);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid green value";
    debug_parsecommand(g);
    sq->g = atoi(g);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid blue value";
    debug_parsecommand(b);
    sq->b = atoi(b);

    size = debug_sizecommand();
    if (size == 0 || size > 3)
        return "Invalid alpha value";
    debug_parsecommand(a);
    sq->a = atoi(a);

    sq->active = 1;

    return "Square drawn";
}

void cmd_init(GameState* state)
{
    debug_initialize();
    debug_addcommand("ping", "Health check", cmd_ping);
    debug_addcommand("draw_square x y width height red green blue alpha", "Draw a square on screen", cmd_draw_square);
    debug_printcommands();
}
