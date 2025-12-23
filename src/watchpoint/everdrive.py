import struct
import time
from typing import Optional, Tuple

from pyftdi.ftdi import Ftdi

# Everdrive Constants
BUFFER_SIZE = 512

# Data types (from usb.h)
DATATYPE_TEXT = 0x01
DATATYPE_RAWBINARY = 0x02
DATATYPE_HEADER = 0x03
DATATYPE_SCREENSHOT = 0x04
DATATYPE_HEARTBEAT = 0x05

# Protocol markers
MARKER_DMA = b"DMA@"
MARKER_CMPH = b"CMPH"


class Everdrive:
    def __init__(self, vendor_id=0x0403, product_id=0x6001):
        self.ftdi = Ftdi()
        self.vendor_id = vendor_id
        self.product_id = product_id
        self.is_open = False

    def open(self):
        try:
            # Open FTDI device
            self.ftdi.open(self.vendor_id, self.product_id, interface=0)

            # Configure for EverDrive communication
            # FT245R in synchronous FIFO mode
            self.ftdi.set_bitmode(0xFF, Ftdi.BitMode.RESET)
            time.sleep(0.05)

            # Set synchronous 245 FIFO mode
            self.ftdi.set_bitmode(0xFF, Ftdi.BitMode.SYNCFF)

            # Configure timeouts
            self.ftdi.set_latency_timer(2)  # 2ms latency

            # Purge buffers
            self.ftdi.purge_buffers()

            self.is_open = True
        except Exception as e:
            raise Exception(f"Failed to open EverDrive: {e}")

    def close(self):
        if self.is_open:
            self.ftdi.close()
            self.is_open = False

    def _align(self, size, alignment):
        return ((size + alignment - 1) // alignment) * alignment

    def send_data(self, datatype: int, data: bytes) -> None:
        if not self.is_open:
            raise Exception("Device not open")

        # Build header: datatype (1 byte) + size (3 bytes, big-endian)
        size = len(data)
        header = struct.pack(">I", (datatype << 24) | (size & 0x00FFFFFF))

        # Build full packet
        packet = MARKER_DMA + header + data + MARKER_CMPH

        # Align to 2 bytes (from UNFLoader source)
        aligned_size = self._align(len(packet), 2)
        if aligned_size > len(packet):
            packet += b"\x00" * (aligned_size - len(packet))

        # Send in chunks
        offset = 0
        while offset < len(packet):
            chunk_size = min(BUFFER_SIZE, len(packet) - offset)
            chunk = packet[offset : offset + chunk_size]

            # Write chunk
            bytes_written = self.ftdi.write_data(chunk)
            if bytes_written != len(chunk):
                raise Exception(
                    f"Write failed: expected {len(chunk)}, wrote {bytes_written}"
                )

            offset += chunk_size

    def receive_data(
        self, timeout: float = 2.0
    ) -> Optional[Tuple[int, bytes]]:
        if not self.is_open:
            raise Exception("Device not open")

        start_time = time.time()

        try:
            # Poll for DMA@ marker with timeout
            marker = b""
            while len(marker) < 4 and (time.time() - start_time) < timeout:
                try:
                    chunk = self.ftdi.read_data_bytes(4 - len(marker))
                    if chunk:
                        marker += chunk
                    else:
                        time.sleep(0.01)  # Small delay if no data
                except Exception:
                    time.sleep(0.01)

            if len(marker) < 4:
                print("Timeout - no data received")
                return None

            if marker != MARKER_DMA:
                print(f"Warning: Expected DMA@, got {marker.hex()}")
                return None

            # Read header (datatype + size)
            header_bytes = self.ftdi.read_data_bytes(4)
            if len(header_bytes) < 4:
                print("Incomplete header received")
                return None

            header = struct.unpack(">I", header_bytes)[0]

            datatype = (header >> 24) & 0xFF
            size = header & 0x00FFFFFF

            print(f"Receiving: datatype={datatype:02X}, size={size}")

            # Read data
            aligned_size = self._align(size, 2)
            data = bytearray()

            while len(data) < aligned_size:
                needed = aligned_size - len(data)
                chunk_size = min(BUFFER_SIZE, needed)
                chunk = self.ftdi.read_data_bytes(chunk_size)
                if len(chunk) == 0:
                    print("Warning: Incomplete data received")
                    break
                data.extend(chunk)
                print(f"  Received {len(data)}/{aligned_size} bytes")

            # Trim to actual size
            data = bytes(data[:size])

            # Read CMPH marker
            marker = self.ftdi.read_data_bytes(4)
            if len(marker) < 4 or marker != MARKER_CMPH:
                print(
                    f"Warning: Expected CMPH, got {marker.hex() if marker else 'nothing'}"
                )

            print("Receive complete!")
            return (datatype, data)

        except Exception as e:
            print(f"Exception during receive: {e}")
            import traceback

            traceback.print_exc()
            return None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
