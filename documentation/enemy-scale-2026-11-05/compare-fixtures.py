"""Convert fixture captures to PNG and compare silhouettes (requires Pillow)."""
from pathlib import Path
from PIL import Image

root = Path(__file__).parent
for path in root.glob('*.bmp'):
    with Image.open(path) as image:
        image.save(path.with_suffix('.png'))
    path.unlink()

for frame in range(2):
    masks = []
    for mode in ('compatibility', 'modern'):
        with Image.open(root / f'enemy-fixture-{mode}-{frame}.png') as image:
            image = image.convert('RGB')
            background = image.getpixel((0, 0))
            masks.append([pixel != background for pixel in image.getdata()])
    differences = sum(a != b for a, b in zip(*masks))
    print(f'Pose {frame}: native={sum(masks[0])}, modern={sum(masks[1])}, '
          f'silhouette differences={differences}')
    assert differences == 0
