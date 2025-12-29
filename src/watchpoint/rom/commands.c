#include "engine.h"


extern GameState state;
char response[0xFF] = {0};

static int extract_int32(char* data, int* offset)
{
    int value = (
        (data[*offset] << 24)     |
        (data[*offset + 1] << 16) |
        (data[*offset + 2] << 8)  |
        (data[*offset + 3])
    );
    *offset += 4;
    return value;
}

static int extract_byte(char* data, int* offset)
{
    int value = (unsigned char)data[*offset];
    *offset += 1;
    return value;
}

static void extract_bytes(char* dest, char* src, int* offset, int length)
{
    for (int i = 0; i < length; i++) {
        dest[i] = src[*offset + i];
    }
    *offset += length;
}

static void extract_string(char* dest, char* src, int* offset)
{
    uint8_t size = extract_byte(src, offset);
    extract_bytes(dest, src, offset, size);
    dest[size] = '\0';
}

void cmd_ping(char* response, char* data, size_t size)
{
    if (size == 0) {
        strcpy(response, "pong");
    } else {
        int offset = 0;
        char msg[0xFF] = {0};
        extract_string(msg, data, &offset);
        strcpy(response, msg);
    }
}

void cmd_draw_square(char* response, char* data, int size)
{
    int x, y, w, h;
    int r, g, b, a;

    /* Packet format:
     * [x:4] [y:4] [w:4] [h:4] [r:1] [g:1] [b:1] [a:1]
     * Total: 20 bytes
     */
    if (size < 20) {
        return;
    }

    int i;
    int offset = 0;

    /* Extract values (big-endian) */
    x = extract_int32(data, &offset);
    y = extract_int32(data, &offset);
    w = extract_int32(data, &offset);
    h = extract_int32(data, &offset);
    r = extract_byte(data, &offset);
    g = extract_byte(data, &offset);
    b = extract_byte(data, &offset);
    a = extract_byte(data, &offset);

    /* Find inactive slot or reuse oldest */
    for (i = 0; i < MAX_SQUARES; i++) {
        if (!state.squares[i].active) {
            break;
        }
    }

    /* If all slots full, use circular buffer */
    if (i >= MAX_SQUARES) {
        i = state.square_idx;
        state.square_idx = (state.square_idx + 1) % MAX_SQUARES;
    }

    /* Store square data */
    state.squares[i].x = x;
    state.squares[i].y = y;
    state.squares[i].w = w;
    state.squares[i].h = h;
    state.squares[i].r = r;
    state.squares[i].g = g;
    state.squares[i].b = b;
    state.squares[i].a = a;
    state.squares[i].active = 1;

    sprintf(state.debug_str, "x:%d, y:%d, w:%d, h:%d\nr:%d, g:%d, b:%d, a:%d", x, y, w, h, r, g, b, a);
    strcpy(response, "drawn");
}

char* process()
{
    state.pkts_received++;

    int cmd, offset = 0;
    int size = debug_sizecommand();

    if (size < 1 || size > 0xF0) {
        return "invalid";
    }

    char data[0xFF] = {0};
    debug_parsecommand(data);
    data[size] = '\0';

    memcpy(state.debug_str, data, size + 1);
    state.last_size = size;

    size_t decoded_size;
    char* decoded_data = (char*)base64_decode(data, size, &decoded_size);

    cmd = extract_int32(decoded_data, &offset);
    strcpy(response, "nothing");

    switch (cmd) {
        case CMD_PING:
            cmd_ping(response, &decoded_data[offset], decoded_size - offset);
            break;

        case CMD_DRAW_SQUARE:
            cmd_draw_square(response, &decoded_data[offset], decoded_size - offset);
            break;

        default:
            break;
    }

    free(decoded_data);
    return response;
}
