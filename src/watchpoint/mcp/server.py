# import struct
# import subprocess
# import sys
# from base64 import b64decode, b64encode
# from typing import Annotated

# from fastmcp import FastMCP

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

#         str_data = b"Hello, World 123!!"
#         data = struct.pack(
#             ">I{}p".format(len(str_data) + 1), CMD_PING, str_data
#         )
#         data = b"cmd " + b64encode(data)

#         # Prepare the input as a string
#         input_data = "cmd AAAAARJJIZWxsbywgV29ybGQgMTIzISE=\nexit\n"

#         # Run the process
#         result = subprocess.run(
#             ["UNFLoader", "-d", "-b"],
#             input=input_data,
#             capture_output=True,
#             text=True,
#         )

#         print("Return code:", result.returncode)
#         print("STDOUT:", result.stdout)
#         print("STDERR:", result.stderr)

#         # if result:
#         #     datatype, data = result
#         #     if datatype == DATATYPE_RAWBINARY and len(data) >= 4:
#         #         response = struct.unpack(">I", data[:4])[0]
#         #         if response == MSG_PONG:
#         #             return f"✓ SUCCESS! N64 responded with PONG (0x{response:08X})"
#         #         else:
#         #             return f"? Unexpected response: 0x{response:08X}"
#         #     else:
#         #         return f"? Unexpected data: type={datatype}, len={len(data)}"
#         # else:
#         #     return "✗ No response from N64 (timeout)"

#     except Exception as e:
#         return f"✗ Error during ping: {e}"


# if __name__ == "__main__":
#     mcp.run()
