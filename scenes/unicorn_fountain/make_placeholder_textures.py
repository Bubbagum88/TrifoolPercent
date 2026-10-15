#!/usr/bin/env python3
"""Write placeholder PNGs for the Unicorn Fountain textures that Triforce% did not publish.

UnicornFountain.blend references nine images under //textures/ that are not in the triforce-percent repo
(they appear to have been ripped from the ROM at build time). Fast64 refuses to export a material whose
image file has 0 size, so this script fills the gaps with simple procedural stand-ins. Any file that
already exists is left alone, so dropping the real textures into the same folder wins.

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


def stone(x, y):
    # Fountain stone: pale limestone-looking blocks, with soft noise.
    rng = (x * 31 + y * 17) % 23
    shade = -10 if rng < 8 else (10 if rng > 18 else 0)
    seam = (x % 32 == 0) or (y % 24 == 0)
    if seam:
        return (90, 86, 76, 255)
    return (200 + shade, 192 + shade, 170 + shade, 255)


def i8_tile(x, y):
    # Generic grayscale decal.
    v = 160 - ((x // 4 + y // 4) % 2) * 20
    return (v, v, v, 255)


def water_rgba(seed, base):
    rng = random.Random(seed)
    noise = [[rng.randint(-24, 24) for _ in range(32)] for _ in range(32)]

    def px(x, y):
        n = noise[y % 32][x % 32]
        r, g, b = base
        return (max(0, min(255, r + n)), max(0, min(255, g + n)), max(0, min(255, b + n)), 255)

    return px


# Every missing image the .blend points at. Sizes are what Fast64 reads from the material UVs; a square
# 32x32 is a safe default where the real size isn't known from the filename.
PLACEHOLDERS = {
    "Fountain_Stone_00.png": (32, 32, stone),
    "00004800_32x64_i8.png": (32, 64, i8_tile),
    "I8_0200B000#ClampY.png": (32, 32, i8_tile),
    "RGBA16_02007000#MirrorY.png": (32, 32, water_rgba(1, (90, 150, 220))),
    "RGBA16_02007810.png": (32, 32, water_rgba(2, (110, 170, 230))),
    "RGBA16_02008000#ClampX.png": (32, 32, water_rgba(3, (130, 180, 230))),
    "RGBA16_02009000.png": (32, 32, water_rgba(4, (100, 160, 220))),
    "RGBA16_0200D800.png": (32, 32, water_rgba(5, (120, 170, 220))),
    "RGBA16_0201CBC0.png": (32, 32, water_rgba(6, (140, 185, 230))),
    "RGBA16_02021BC0.png": (32, 32, water_rgba(7, (115, 165, 215))),
}


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 1
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for name, (width, height, pixel) in PLACEHOLDERS.items():
        path = out / name
        if path.exists() and path.stat().st_size > 0:
            continue
        write_png(path, width, height, pixel)
        print(f"placeholder: {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
