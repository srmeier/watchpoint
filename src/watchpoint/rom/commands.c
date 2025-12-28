#include "engine.h"

#define CMD_PING 0x01

extern GameState state;

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

void cmd_ping()
{
    uint32_t pong = MSG_PONG;
    usb_write(DATATYPE_RAWBINARY, &pong, sizeof(pong));
}

void cmd_draw_square(char* data, int size)
{
    int x, y, w, h;
    int r, g, b, a;

    /* Packet format:
     * [CMD:1] [x:4] [y:4] [w:4] [h:4] [r:1] [g:1] [b:1] [a:1]
     * Total: 21 bytes
     */
    if (size < 21) {
        return;
    }

    int i;
    int offset = 1;

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
}

void cmd_process(char* data, int size)
{
    int cmd, offset = 0;

    if (size < 1) {
        return;
    }

    cmd = extract_int32(data, &offset);

    switch (cmd) {
        case CMD_PING:
            cmd_ping();

            extract_string(state.debug_str, data, &offset);

            break;

        case CMD_DRAW_SQUARE:
            cmd_draw_square(data, size);
            break;

        default:
            break;
    }
}

void cmd_poll()
{
    uint32_t header;
    uint8_t datatype;
    uint32_t size;
    char command_buffer[512];

    // Check for USB commands
    header = usb_poll();
    if (header != 0)
    {
        datatype = USBHEADER_GETTYPE(header);
        size = USBHEADER_GETSIZE(header);

        // Track last received values for debugging
        state.last_datatype = datatype;
        state.last_size = size;

        // Handle binary commands
        if (datatype == DATATYPE_RAWBINARY && size <= sizeof(command_buffer))
        {
            state.pkts_received++;

            usb_read(command_buffer, size);
            cmd_process(command_buffer, size);
        }
        else
        {
            // Unknown data, skip it
            usb_skip(size);
        }
    }
}

void cmd_init()
{
    usb_initialize();
}
