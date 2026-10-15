#!/usr/bin/env python3
"""Write placeholder PNGs for the Triforce Room textures that Triforce% did not publish.

TriforceRoom.blend points at five images under //textures/ that are not in the
triforce-percent repository (they were ripped from the ROM or from beta material).
Fast64 refuses to export a material whose image is missing, so this script fills
the gaps with simple procedural stand-ins. Any file that already exists is left
alone, so dropping the real textures into the same folder wins.

Usage: make_placeholder_textures.py <textures dir>
"""

import random
import struct
import sys
import zlib
from pathlib import Path


def write_png(path: Path, width: int, height: int, pixel) -> None:
    rows = bytearray()
    for y in range(height):
        rows.append(0)  # filter: none
        for x in range(width):
            rows.extend(pixel(x, y))

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(rows), 9))
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


def bricks(x, y):
    # Sandstone bricks, 16x8 with a half-brick offset every other row.
    row = y // 8
    bx = (x + (8 if row % 2 else 0)) % 16
    mortar = y % 8 == 7 or bx == 15
    if mortar:
        return (96, 78, 52, 255)
    shade = 20 if (x * 7 + y * 13) % 11 < 3 else 0
    return (196 - shade, 164 - shade, 104 - shade, 255)


def tiles(x, y):
    # Pale floor tiles, 16x16 with dark grout.
    if x % 16 == 0 or y % 16 == 0:
        return (70, 70, 80, 255)
    light = ((x // 16) + (y // 16)) % 2
    return (200, 196, 180, 255) if light else (180, 176, 164, 255)


def water(seed, base):
    rng = random.Random(seed)
    noise = [[rng.randint(-24, 24) for _ in range(32)] for _ in range(32)]

    def px(x, y):
        n = noise[y % 32][x % 32]
        r, g, b = base
        return (max(0, min(255, r + n)), max(0, min(255, g + n)), max(0, min(255, b + n)), 255)

    return px


PLACEHOLDERS = {
    "Beta_Triforce_Bricks_00.png": (32, 32, bricks),
    "Tile2.png": (32, 32, tiles),
    "COS-0x10F08_32x32_RGBA16.png": (32, 32, water(1, (60, 110, 200))),
    "000170B8_32x32_rgba16.png": (32, 32, water(2, (120, 170, 230))),
    "Brick.bmp": None,  # referenced by the .blend but used by no material
}


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 1
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for name, spec in PLACEHOLDERS.items():
        path = out / name
        if spec is None or path.exists():
            continue
        width, height, pixel = spec
        write_png(path, width, height, pixel)
        print(f"placeholder: {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
