# import struct
# import sys
# from typing import Annotated

# from fastmcp import FastMCP

# from watchpoint.mcp.everdrive import (
#     DATATYPE_RAWBINARY,
#     DATATYPE_TEXT,
#     Everdrive,
# )

# mcp = FastMCP("Watchpoint")

# # Command IDs
# CMD_PING = 0x01
# CMD_DRAW_SQUARE = 0x02


# @mcp.tool
# def ping() -> str:
#     """Ping the server"""
#     try:
#         # Send PING
#         print("Sending PING...")

#         ping_bytes = struct.pack(">I", MSG_PING)
#         with Everdrive() as ed:
#             ed.send_data(DATATYPE_RAWBINARY, ping_bytes)

#             # Wait for PONG
#             print("Waiting for PONG...")
#             result = ed.receive_data(timeout=5.0)

#         if result:
#             datatype, data = result
#             if datatype == DATATYPE_RAWBINARY and len(data) >= 4:
#                 response = struct.unpack(">I", data[:4])[0]
#                 if response == MSG_PONG:
#                     return f"✓ SUCCESS! N64 responded with PONG (0x{response:08X})"
#                 else:
#                     return f"? Unexpected response: 0x{response:08X}"
#             else:
#                 return f"? Unexpected data: type={datatype}, len={len(data)}"
#         else:
#             return "✗ No response from N64 (timeout)"

#     except Exception as e:
#         return f"✗ Error during ping: {e}"


# if __name__ == "__main__":
#     mcp.run()
