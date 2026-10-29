#!/usr/bin/env python3
"""Create an atomic, versioned static browser release from a completed build."""
import argparse
import hashlib
import json
import pathlib
import shutil
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--runtime', type=pathlib.Path, default=pathlib.Path('runtime'))
parser.add_argument('--output', type=pathlib.Path, required=True)
args = parser.parse_args()
files = [args.runtime / ('tankgame-linux.' + ext) for ext in ('html', 'js', 'wasm', 'data')]
for file in files:
    if not file.is_file():
        parser.error(f'Missing build artifact: {file}')
manifest = {file.name: hashlib.sha256(file.read_bytes()).hexdigest() for file in files}
version = hashlib.sha256(json.dumps(manifest, sort_keys=True).encode()).hexdigest()[:16]
destination = args.output / version
args.output.mkdir(parents=True, exist_ok=True)
if destination.exists():
    for name, digest in manifest.items():
        if hashlib.sha256((destination / name).read_bytes()).hexdigest() != digest:
            raise RuntimeError(f'Existing release is corrupt: {destination / name}')
else:
    with tempfile.TemporaryDirectory(dir=args.output) as staging:
        staged = pathlib.Path(staging) / version
        staged.mkdir()
        for file in files:
            shutil.copyfile(file, staged / file.name)
        (staged / 'manifest.json').write_text(json.dumps({'version': version, 'sha256': manifest}, indent=2) + '\n')
        staged.rename(destination)
print(destination / 'tankgame-linux.html')
