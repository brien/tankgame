"""Build the Linux close-up fixture from an existing native CMake build.

Run from the repository root with the same development dependency environment
used for the game build. Output: build/overlay-fixture. No production source changes.
"""
from pathlib import Path
import shlex
import subprocess

root = Path.cwd()
link = shlex.split((root / 'build/src/CMakeFiles/tankgame-linux.dir/link.txt').read_text())
objects = [str(root / 'build/src' / x) for x in link
           if x.endswith('.o') and '/main.cpp.o' not in x]
libraries = link[link.index('-o') + 2:]
(root / 'build').mkdir(exist_ok=True)
subprocess.run([link[0], '-std=c++14', '-O2', '-iquote' + str(root / 'src'),
                str(Path(__file__).with_name('overlay-fixture.cpp')), *objects,
                *libraries, '-o', str(root / 'build/overlay-fixture')], check=True)
