#include "engine.h"


extern GameState state;
char response[0xFF] = {0};

static int extract_int32(unsigned char* data, int* offset)
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

static unsigned int extract_uint32(unsigned char* data, int* offset)
{
    return (unsigned int)extract_int32(data, offset);
}

static unsigned char extract_byte(unsigned char* data, int* offset)
{
    unsigned char value = data[*offset];
    *offset += 1;
    return value;
}

static void extract_bytes(char* dest, unsigned char* src, int* offset, int length)
{
    for (int i = 0; i < length; i++) {
        dest[i] = src[*offset + i];
    }
    *offset += length;
}

static void extract_string(char* dest, unsigned char* src, int* offset)
{
    uint8_t size = extract_byte(src, offset);
    extract_bytes(dest, src, offset, size);
    dest[size] = '\0';
}

void cmd_ping(char* response, unsigned char* data, size_t size)
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

void cmd_draw_square(char* response, unsigned char* data, int size)
{
    int x, y;
    unsigned int w, h, r, g, b, a;

    /* Packet format:
     * [x:4] [y:4] [w:4] [h:4] [r:4] [g:4] [b:4] [a:4]
     * Total: 32 bytes
     */
    if (size < 32) {
        strcpy(response, "wrong number of bytes");
        return;
    }

    int i;
    int offset = 0;

    /* Extract values (big-endian) */
    x = extract_int32(data, &offset);
    y = extract_int32(data, &offset);
    w = extract_uint32(data, &offset);
    h = extract_uint32(data, &offset);
    r = extract_uint32(data, &offset);
    g = extract_uint32(data, &offset);
    b = extract_uint32(data, &offset);
    a = extract_uint32(data, &offset);

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

    sprintf(state.debug_str, "x:%d, y:%d, w:%u, h:%u\nr:%u, g:%u, b:%u, a:%u", x, y, w, h, r, g, b, a);
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

    sprintf(state.debug_str, "Data: %s", data);
    state.last_size = size;

    size_t decoded_size;
    unsigned char* decoded_data = base64_decode(data, size, &decoded_size);

    cmd = extract_byte(decoded_data, &offset);
    strcpy(response, "nothing");
    state.last_cmd = cmd;

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
