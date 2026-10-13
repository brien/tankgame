"""Convert SDL's BMP captures losslessly to PNG and compare renderer output."""
from pathlib import Path
import struct, zlib, json
root = Path(__file__).resolve().parent

def load(p):
    data=p.read_bytes()
    offset=struct.unpack_from('<I',data,10)[0]
    width,height=struct.unpack_from('<ii',data,18)
    bits=struct.unpack_from('<H',data,28)[0]
    assert bits==32 and height>0
    rows=[]
    for y in range(height):
        bgra=data[offset+(height-1-y)*width*4:offset+(height-y)*width*4]
        rgb=bytearray(width*3)
        rgb[0::3]=bgra[2::4];rgb[1::3]=bgra[1::4];rgb[2::3]=bgra[0::4]
        rows.append(bytes(rgb))
    def chunk(k,d):return struct.pack('!I',len(d))+k+d+struct.pack('!I',zlib.crc32(k+d)&0xffffffff)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('!2I5B',width,height,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(b''.join(b'\0'+r for r in rows)))+chunk(b'IEND',b'')
    p.with_suffix('.png').write_bytes(png)
    return b''.join(rows)

results=[]
for frame in range(6):
    a=load(root/f'fixture-compatibility-{frame}.bmp')
    b=load(root/f'fixture-modern-{frame}.bmp')
    diff=[abs(x-y) for x,y in zip(a,b)]
    pixels=sum(a[i:i+3]!=b[i:i+3] for i in range(0,len(a),3))
    masks=sum((a[i:i+3]!=b'\0\0\x33')!=(b[i:i+3]!=b'\0\0\x33') for i in range(0,len(a),3))
    large_differences=sum(max(diff[i:i+3])>1 for i in range(0,len(diff),3))
    results.append(dict(pixelsAboveOneChannelValue=large_differences,frame=frame,differingPixels=pixels,silhouetteDifferences=masks,maxChannelDifference=max(diff),meanChannelDifference=sum(diff)/len(diff)))
(root/'fixture-comparison.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

# Permit one edge-coverage pixel per pose and single-value rounding elsewhere.
assert all(r["silhouetteDifferences"] <= 1 and r["pixelsAboveOneChannelValue"] <= 1 for r in results), "Opaque lighting parity regression"
