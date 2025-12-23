import struct
import sys
from typing import Annotated

from fastmcp import FastMCP

from watchpoint.everdrive import DATATYPE_RAWBINARY, DATATYPE_TEXT, Everdrive

mcp = FastMCP("Watchpoint")

# PING/PONG messages
MSG_PING = 0x50494E47
MSG_PONG = 0x504F4E47

# Command IDs
CMD_ADD_SQUARE = 0x01
CMD_ECHO = 0x02


@mcp.tool
def add_square(
    x: Annotated[int, "horizontal position (-160 to 160, 0 = center)"] = 0,
    y: Annotated[int, "vertical position (-120 to 120, 0 = center)"] = 0,
    z: Annotated[int, "depth (-10 to 10, negative = closer)"] = -5,
    size: Annotated[int, "size in pixels (typical: 20-100)"] = 50,
    rotation: Annotated[int, "rotation in degrees (0-360)"] = 0,
    r: Annotated[int, "red component (0-255)"] = 255,
    g: Annotated[int, "green component (0-255)"] = 0,
    b: Annotated[int, "blue component (0-255)"] = 0,
    a: Annotated[int, "alpha component (0-255)"] = 255,
) -> str:
    """
    Add a colored square to the N64 game scene.

    The square will be rendered with the specified position, size, rotation, and color.
    Coordinates are relative to screen center (320x240 screen):
    - x: horizontal position (-160 to 160, 0 = center)
    - y: vertical position (-120 to 120, 0 = center)
    - z: depth (-10 to 10, negative = closer)

    Size is in pixels (typical: 20-100).
    Rotation is in degrees (0-360).
    Colors are RGBA values (0-255).
    """
    try:
        # Validate ranges
        x = max(-160, min(160, x))
        y = max(-120, min(120, y))
        z = max(-10, min(10, z))
        size = max(10, min(200, size))
        rotation = rotation % 360
        r = max(0, min(255, r))
        g = max(0, min(255, g))
        b = max(0, min(255, b))
        a = max(0, min(255, a))

        # Build packet: [CMD:1][x:4][y:4][z:4][size:4][rotation:4][r:1][g:1][b:1][a:1]
        packet = struct.pack(
            ">Biiiii", CMD_ADD_SQUARE, x, y, z, size, rotation
        )
        packet += struct.pack("BBBB", r, g, b, a)

        # Pad to even length (25 bytes -> 26 bytes)
        if len(packet) % 2 == 1:
            packet += b"\x00"

        print(
            f"Sending add_square: pos=({x},{y},{z}) size={size} rot={rotation} color=({r},{g},{b},{a})",
            file=sys.stderr,
        )

        # Send to N64
        with Everdrive() as ed:
            ed.send_data(DATATYPE_RAWBINARY, packet)

        return f"✓ Added square at ({x}, {y}, {z}), size={size}, rotation={rotation}°, color=RGBA({r},{g},{b},{a})"

    except Exception as e:
        return f"✗ Error during add_square: {e}"


@mcp.tool
def ping() -> str:
    """Ping the server"""
    try:
        # Send PING
        print("Sending PING...")

        ping_bytes = struct.pack(">I", MSG_PING)
        with Everdrive() as ed:
            ed.send_data(DATATYPE_RAWBINARY, ping_bytes)

            # Wait for PONG
            print("Waiting for PONG...")
            result = ed.receive_data(timeout=5.0)

        if result:
            datatype, data = result
            if datatype == DATATYPE_RAWBINARY and len(data) >= 4:
                response = struct.unpack(">I", data[:4])[0]
                if response == MSG_PONG:
                    return f"✓ SUCCESS! N64 responded with PONG (0x{response:08X})"
                else:
                    return f"? Unexpected response: 0x{response:08X}"
            else:
                return f"? Unexpected data: type={datatype}, len={len(data)}"
        else:
            return "✗ No response from N64 (timeout)"

    except Exception as e:
        return f"✗ Error during ping: {e}"


@mcp.tool
def echo(message: str) -> str:
    """Echo a message to the server and receive it back"""

    try:
        # Get message parameter
        print(
            f"DEBUG: echo tool received message: '{message}' (type: {type(message)})"
        )

        if not message:
            return "✗ No message provided"

        # Limit to 255 chars
        message = message[:255]
        message_bytes = message.encode("utf-8")
        msg_len = len(message_bytes)

        # Build echo packet: [CMD:1][length:1][string:N]
        packet = struct.pack("BB", CMD_ECHO, msg_len) + message_bytes

        # Pad to even length for 2-byte alignment
        if len(packet) % 2 == 1:
            packet += b"\x00"

        print(
            f"DEBUG: Encoded to {msg_len} bytes: {message_bytes.hex()}",
            file=sys.stderr,
        )
        print(
            f"DEBUG: Full packet ({len(packet)} bytes, padded): {packet.hex()}",
            file=sys.stderr,
        )

        with Everdrive() as ed:
            # Send to N64
            print(f"Sending ECHO: '{message}'")
            ed.send_data(DATATYPE_RAWBINARY, packet)

            # Wait for echo response
            print("Waiting for echo...")
            result = ed.receive_data(timeout=5.0)

        if result:
            datatype, data = result
            if datatype == DATATYPE_TEXT:
                echo_text = data.decode("utf-8", errors="ignore")
                if echo_text == message:
                    return f"✓ SUCCESS! N64 echoed back: '{echo_text}'"
                else:
                    return f"? Mismatch! Sent: '{message}', Got: '{echo_text}'"
            else:
                return f"? Unexpected data type: {datatype}"
        else:
            return "✗ No response from N64 (timeout)"

    except Exception as e:
        return f"✗ Error during echo: {e}"


if __name__ == "__main__":
    mcp.run()
