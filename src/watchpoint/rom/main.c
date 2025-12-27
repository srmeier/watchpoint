#include "engine.h"


GameState state = {0};

int main(void)
{
    display_init(
        RESOLUTION_320x240,
        DEPTH_16_BPP,
        2,
        GAMMA_NONE,
        FILTERS_DISABLED
    );

    dfs_init(DFS_DEFAULT_LOCATION);
    joypad_init();

    cmd_init();

    while(1)
    {
        update();
        render();
    }
}
