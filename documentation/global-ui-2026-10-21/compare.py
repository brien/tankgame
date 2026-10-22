"""Compare fixture BMPs and retain lossless PNGs; uses only the Python stdlib."""
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

report = []
for source in sorted(root.glob('compatibility-*.bmp')):
    target = root/source.name.replace('compatibility', 'modern')
    width, height, a = read_bmp(source)
    w2, h2, b = read_bmp(target)
    assert (width,height) == (w2,h2)
    delta = [abs(x-y) for x,y in zip(a,b)]
    report.append({'case':source.stem.removeprefix('compatibility-'),
                   'max_channel_difference':max(delta), 'different_channels':sum(x!=0 for x in delta)})
    if source.stem.endswith('-0'):
        png(source.with_suffix('.png'),width,height,a)
        png(target.with_suffix('.png'),width,height,b)
assert len(report) == 6
(root/'comparison.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
assert all(row['max_channel_difference'] <= 1 for row in report)
