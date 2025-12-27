#include "engine.h"


extern GameState state;

void render()
{
    char tStr[256];
    static display_context_t disp = 0;

    disp = display_get();
    graphics_fill_screen(disp, 0);

    // Draw FPS
    sprintf(tStr, "FPS: %.2f", state.fps);
    graphics_draw_text(disp, 5, 5, tStr);

    // Draw squares
    for (int i = 0; i < MAX_SQUARES; i++) {
        if (state.squares[i].active) {
            graphics_draw_box(
                disp,
                state.squares[i].x,
                state.squares[i].y,
                state.squares[i].w,
                state.squares[i].h,
                graphics_make_color(
                    state.squares[i].r,
                    state.squares[i].g,
                    state.squares[i].b,
                    state.squares[i].a
                )
            );
        }
    }

    display_show(disp);
}
