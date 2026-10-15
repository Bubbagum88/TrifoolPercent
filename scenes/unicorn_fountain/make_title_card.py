#!/usr/bin/env python3
"""Write the "Unicorn Fountain" place-name title card.

Debug scenes like Test01, where the fountain lives, have no place name. The TrifoolPercent module shows this
texture as the room's title card on entry (a 144x24 IA8 texture, the vanilla title card format). Triforce%'s
own title.ia8.png ships as 144x48 (two stacked frames); we take the top half.

Usage: make_title_card.py <out dir> <source title.ia8.png>
"""

import struct
import sys
from pathlib import Path

from PIL import Image

WIDTH, HEIGHT = 144, 24
RESOURCES = ["textures/trifool_place_title_cards/gTrifoolUnicornFountainTitleCardTex"]

# libultraship resource header (0x40 bytes): little endian, type 'OTEX', version 0, id 0xDEADBEEFDEADBEEF.
HEADER = struct.pack("<I4sIQ", 0, b"XETO", 0, 0xDEADBEEFDEADBEEF).ljust(0x40, b"\0")
TEXTURE_TYPE_IA8 = 8


def render(source: str) -> bytes:
    """Read the shipped title PNG, crop to 144x24, and pack as IA8 (one byte per pixel, 4 bits each)."""
    image = Image.open(source).convert("LA")
    if image.size != (WIDTH, HEIGHT):
        image = image.crop((0, 0, WIDTH, HEIGHT)) if image.height > HEIGHT else image.resize((WIDTH, HEIGHT))
    pixels = bytearray()
    for luminance, alpha in image.getdata():
        pixels.append(((luminance >> 4) << 4) | (alpha >> 4))
    return bytes(pixels)


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    out = Path(sys.argv[1])
    pixels = render(sys.argv[2])
    resource = HEADER + struct.pack("<IIII", TEXTURE_TYPE_IA8, WIDTH, HEIGHT, len(pixels)) + pixels
    for path in RESOURCES:
        (out / path).parent.mkdir(parents=True, exist_ok=True)
        (out / path).write_bytes(resource)
    print(f"Wrote the Unicorn Fountain title card to {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
