#!/usr/bin/env python3
"""Write the "Triforce Room" place-name title card.

Debug scenes like Besitu, where the room lives, have no place name. The TrifoolPercent module shows this texture
as the room's title card on entry (a 144x24 IA8 texture, the vanilla title card format), so it only appears in a
Ship of Harkinian build with the module.

Usage: make_title_card.py <out dir> [font.ttf]
"""

import struct
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

TEXT = "Triforce Room"
WIDTH, HEIGHT = 144, 24
DEFAULT_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
RESOURCES = ["textures/trifool_place_title_cards/gTrifoolTriforceRoomTitleCardTex"]

# libultraship resource header (0x40 bytes): little endian, type 'OTEX', version 0, id 0xDEADBEEFDEADBEEF.
HEADER = struct.pack("<I4sIQ", 0, b"XETO", 0, 0xDEADBEEFDEADBEEF).ljust(0x40, b"\0")
TEXTURE_TYPE_IA8 = 8


def render(font_path: str) -> bytes:
    """IA8 pixels: white text with a dark outline, like the vanilla cards."""
    size = HEIGHT
    while True:
        font = ImageFont.truetype(font_path, size)
        left, top, right, bottom = font.getbbox(TEXT)
        if right - left <= WIDTH - 6 and bottom - top <= HEIGHT - 6:
            break
        size -= 1
    text = Image.new("L", (WIDTH, HEIGHT))
    ImageDraw.Draw(text).text(
        ((WIDTH - (right - left)) // 2 - left, (HEIGHT - (bottom - top)) // 2 - top), TEXT, 255, font=font
    )
    outline = text.filter(ImageFilter.MaxFilter(5))
    pixels = bytearray()
    for glyph, edge in zip(text.getdata(), outline.getdata()):
        intensity = glyph >> 4
        alpha = max(glyph, edge) >> 4
        pixels.append((intensity << 4) | alpha)
    return bytes(pixels)


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    out = Path(sys.argv[1])
    pixels = render(sys.argv[2] if len(sys.argv) > 2 else DEFAULT_FONT)
    resource = HEADER + struct.pack("<IIII", TEXTURE_TYPE_IA8, WIDTH, HEIGHT, len(pixels)) + pixels
    for path in RESOURCES:
        (out / path).parent.mkdir(parents=True, exist_ok=True)
        (out / path).write_bytes(resource)
    print(f"Wrote the Triforce Room title card to {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
