"""Convert fixture BMPs to lossless PNGs; uses only the Python stdlib."""
from pathlib import Path
import json
import struct
import zlib

root = Path(__file__).resolve().parent

def read_bmp(path):
    data = path.read_bytes()
    offset = struct.unpack_from('<I', data, 10)[0]
    width, height = struct.unpack_from('<ii', data, 18)
    assert struct.unpack_from('<H', data, 28)[0] == 32
    rows = []
    for y in reversed(range(height)):
        row = data[offset+y*width*4:offset+(y+1)*width*4]
        rows.append(bytes(channel for x in range(width) for channel in
                          (row[x*4+2], row[x*4+1], row[x*4])))
    return width, height, b''.join(rows)

def png(path, width, height, rgb):
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind+data))
    rows = b''.join(b'\0'+rgb[y*width*3:(y+1)*width*3] for y in range(height))
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB',width,height,8,2,0,0,0)) +
                     chunk(b'IDAT',zlib.compress(rows)) + chunk(b'IEND',b''))

for source in sorted(root.glob("modern-*.bmp")):
    png(source.with_suffix(".png"), *read_bmp(source))
