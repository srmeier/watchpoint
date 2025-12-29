import random  # noqa: F401
import string  # noqa: F401
import struct
from base64 import b64encode
from time import sleep

from pyftdi.ftdi import Ftdi

ftdi = Ftdi()
ftdi.open(0x0403, 0x6001, interface=0)
ftdi.reset(usb_reset=True)

ftdi.set_bitmode(0xFF, Ftdi.BitMode.RESET)
sleep(0.05)
ftdi.set_bitmode(0xFF, Ftdi.BitMode.SYNCFF)
ftdi.set_latency_timer(128)
ftdi.purge_buffers()

while not ftdi.get_cts():
    print(f"Not Ready: {ftdi.modem_status()}")
    sleep(0.1)

# NOTE: Max 0xF0 characters in the "cmd" string argument and, based on my out-of-band protocol, I can send up to 0xB0 bytes of data

# CMD_PING = 0x01
# str_data = "".join(random.choices(string.ascii_letters + string.digits, k=10)).encode()
# print(str_data)
# # NOTE: Custom pong response to verify data integrity
# data = struct.pack(f">I{len(str_data)+1}p", CMD_PING, str_data)
# # # NOTE: Just a pong response to verify connectivity
# # data = struct.pack(f">I", CMD_PING)

CMD_DRAW_SQUARE = 0x02
data = struct.pack(
    ">IiiiiBBBB", CMD_DRAW_SQUARE, 50, 100, 50, 50, 255, 0, 0, 255
)

data = b64encode(data)

print(len(data))
print(data)

data = b"cmd " + data + b"\x00"
data = struct.pack(
    ">{}s{}".format(len(data), "" if len(data) % 2 == 0 else "x"), data
)
header = struct.pack(">I", (0x01 << 24) | (len(data) & 0x00FFFFFF))

ftdi.write_data(b"DMA@" + header)
for i in range(0, len(data), 64):
    ftdi.write_data(data[i : i + 64])
ftdi.write_data(b"CMPH")

while ftdi.get_dsr():
    response = ftdi.read_data(4)

    if response == b"DMA@":
        header_bytes = ftdi.read_data(4)
        header = struct.unpack(">I", header_bytes)[0]

        datatype = (header >> 24) & 0xFF
        size = header & 0x00FFFFFF

        num_bytes_read = 0
        data = b""

        while num_bytes_read < size:
            chunk = ftdi.read_data(size - num_bytes_read)
            data += chunk
            num_bytes_read += len(chunk)
    if response == b"CMPH":
        print("Received CMPH, done.")
        break

    sleep(0.5)
    print(ftdi.modem_status())

print(f"Received data (type={datatype}, size={size}): {data}")

ftdi.close()
