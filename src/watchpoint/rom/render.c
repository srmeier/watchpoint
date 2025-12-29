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

    // USB Debug
    sprintf(tStr, "USB Pkts: %d", state.pkts_received);
    graphics_draw_text(disp, 5, 20, tStr);

    // USB Header Debug
    sprintf(tStr, "Size: %d, Cmd: %d", state.last_size, state.last_cmd);
    graphics_draw_text(disp, 5, 35, tStr);

    // Debug String
    graphics_draw_text(disp, 5, 50, state.debug_str);

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
