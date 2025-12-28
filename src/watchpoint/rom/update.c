#include "engine.h"

extern GameState state;

void update()
{
    // Handle input
    joypad_poll();
    state.btns = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    // Update FPS
    state.fps = display_get_fps();
}
