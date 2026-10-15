#!/usr/bin/env python3
"""Stage the Triforce% Triforce actor objects for Torch, HarbourMasters' asset converter.

Torch turns ROM data described by YAML into the same binary resources SoH extracts from a real ROM. It identifies
its input by SHA-1 and needs every texture, palette, vertex array, limb and display list declared. So this script:

1. builds a small stand-in "ROM": a 0x1000-byte header followed by the three Triforce% object files
   (actor/<Actor>/zobj.zobj), each 0x1000-aligned;
2. walks each object's display lists and skeletons (F3DEX2) to find everything they reference;
3. writes config.yml keyed by the stand-in's hash, and one YAML per object.

The entry points come from each actor's obj.h in the Triforce% repo.

Usage: make_torch_input.py <triforce-percent checkout> <out dir>
Then:  torch o2r <out dir>/triforce_objects.z64 -s <out dir> -d <resources dir>
"""

import hashlib
import struct
import sys
from pathlib import Path

HEADER_SIZE = 0x1000
ALIGN = 0x1000

# Torch output folder, Triforce% actor folder, display lists, skeletons as (name, offset, skel type), animations
OBJECTS = [
    {
        "folder": "object_trifool_triforce_piece",
        "actor": "TriforcePiece",
        "dlists": [("gTrifoolTriforcePieceDL", 0x0788)],
        "skeletons": [],
        "animations": [],
    },
    {
        "folder": "object_trifool_triforce_chest",
        "actor": "TriforceChest",
        "dlists": [],
        "skeletons": [("gTrifoolTriforceChestSkel", 0x19B0, "Normal")],
        "animations": [("gTrifoolTriforceChestOpenAnim", 0x1A3C)],
    },
    {
        "folder": "object_trifool_triforce_light",
        "actor": "TriforceLight",
        "dlists": [],
        "skeletons": [("gTrifoolTriforceLightSkel", 0x0108, "Flex")],
        "animations": [("gTrifoolTriforceLightAnim", 0x0470)],
    },
]

G_IM_FMT = {0: "RGBA", 1: "YUV", 2: "CI", 3: "IA", 4: "I"}
G_IM_SIZ = {0: 4, 1: 8, 2: 16, 3: 32}


def seg6(addr: int) -> int | None:
    return addr & 0x00FFFFFF if (addr >> 24) == 0x06 else None


class ObjectScan:
    def __init__(self, data: bytes, prefix: str):
        self.data = data
        self.prefix = prefix
        self.dlists: dict[int, str] = {}
        self.vtx: dict[int, int] = {}  # offset -> vertex count
        self.textures: dict[int, dict] = {}
        self.limbs: list[tuple[str, int]] = []
        self.limb_tables: list[tuple[str, int]] = []

    def u32(self, off: int) -> int:
        return struct.unpack_from(">I", self.data, off)[0]

    def add_dlist(self, off: int, name: str | None = None) -> None:
        if off in self.dlists:
            return
        self.dlists[off] = name or f"{self.prefix}DL_{off:06X}"
        self.walk_dlist(off)

    def walk_dlist(self, off: int) -> None:
        timg = None  # (addr, fmt, siz) of the last G_SETTIMG
        loaded = None  # texture address loaded into TMEM, waiting for its render tile size
        tile0 = None  # (fmt, siz) of render tile 0
        tlut = None
        pending_ci = []
        while off + 8 <= len(self.data):
            w0, w1 = struct.unpack_from(">II", self.data, off)
            op = w0 >> 24
            off += 8
            if op == 0x01:  # G_VTX
                count = (w0 >> 12) & 0xFF
                addr = seg6(w1)
                if addr is not None:
                    self.vtx[addr] = max(self.vtx.get(addr, 0), count)
            elif op == 0xDE:  # G_DL
                addr = seg6(w1)
                if addr is not None:
                    self.add_dlist(addr)
                if (w0 >> 16) & 0xFF == 1:  # branch: no return
                    break
            elif op == 0xDF:  # G_ENDDL
                break
            elif op == 0xFD:  # G_SETTIMG
                timg = (w1, (w0 >> 21) & 7, (w0 >> 19) & 3)
            elif op == 0xF5:  # G_SETTILE
                if (w1 >> 24) & 7 == 0:
                    tile0 = ((w0 >> 21) & 7, (w0 >> 19) & 3)
            elif op in (0xF3, 0xF4):  # G_LOADBLOCK / G_LOADTILE
                loaded = timg[0] if timg else None
            elif op == 0xF0:  # G_LOADTLUT
                addr = seg6(timg[0]) if timg else None
                if addr is not None:
                    colors = ((w1 >> 14) & 0x3FF) + 1
                    side = 16 if colors > 16 else 4
                    self.textures[addr] = {"format": "RGBA16", "width": side, "height": colors // side}
                    tlut = addr
            elif op == 0xF2:  # G_SETTILESIZE
                if (w1 >> 24) & 7 == 0 and loaded is not None and tile0 is not None:
                    addr = seg6(loaded)
                    if addr is not None:
                        width = (((w1 >> 12) & 0xFFF) >> 2) + 1
                        height = ((w1 & 0xFFF) >> 2) + 1
                        fmt = f"{G_IM_FMT[tile0[0]]}{G_IM_SIZ[tile0[1]]}"
                        self.textures[addr] = {"format": fmt, "width": width, "height": height}
                        if fmt.startswith("CI"):
                            pending_ci.append(addr)
                    loaded = None
        for addr in pending_ci:
            if tlut is not None:
                self.textures[addr]["tlut"] = tlut

    def add_skeleton(self, off: int, flex: bool, name: str) -> None:
        limb_table = seg6(self.u32(off))
        count = self.data[off + 4]
        self.limb_tables.append((f"{name}Limbs", limb_table))
        for i in range(count):
            limb = seg6(self.u32(limb_table + 4 * i))
            self.limbs.append((f"{name}Limb_{limb:06X}", limb))
            dl = self.u32(limb + 8)
            if seg6(dl) is not None:
                self.add_dlist(seg6(dl))


def yaml_object(obj: dict, scan: ObjectScan, base: int) -> str:
    lines = [":config:", "  segments:", f"    - [ 6, 0x{base:08X} ]", ""]

    def entry(name: str, fields: list[str]) -> None:
        lines.append(f"{name}:")
        lines.extend(f"  {field}" for field in fields)
        lines.append(f"  symbol: {name}")
        lines.append("")

    for name, off, skel_type in obj["skeletons"]:
        entry(name, ["type: OOT:SKELETON", f"offset: 0x{off:04X}", "limb_type: Standard", f"skel_type: {skel_type}"])
    for name, off in scan.limb_tables:
        entry(name, ["type: BLOB", f"offset: 0x{off:04X}", "size: 0"])
    for name, off in scan.limbs:
        entry(name, ["type: OOT:LIMB", f"offset: 0x{off:04X}", "limb_type: Standard"])
    for name, off in obj["animations"]:
        entry(name, ["type: OOT:ANIMATION", f"offset: 0x{off:04X}"])
    for off, name in sorted(scan.dlists.items()):
        entry(name, ["type: GFX", f"offset: 0x{off:04X}"])
    for off, count in sorted(scan.vtx.items()):
        entry(f"{scan.prefix}Vtx_{off:06X}",
              ["type: OOT:ARRAY", f"offset: 0x{off:04X}", f"count: {count}", "array_type: VTX"])
    for off, tex in sorted(scan.textures.items()):
        fields = ["type: TEXTURE", f"offset: 0x{off:04X}", f"format: {tex['format']}",
                  f"width: {tex['width']}", f"height: {tex['height']}"]
        if "tlut" in tex:
            fields.append(f"tlut: 0x{tex['tlut']:04X}")
        entry(f"{scan.prefix}Tex_{off:06X}", fields)
    return "\n".join(lines)


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 1
    tfp = Path(sys.argv[1])
    out = Path(sys.argv[2])
    (out / "triforce" / "objects").mkdir(parents=True, exist_ok=True)

    rom = bytearray(HEADER_SIZE)
    rom[0x20:0x34] = b"TRIFOOLPERCENT".ljust(0x14, b" ")
    staged = []
    for obj in OBJECTS:
        data = (tfp / "actor" / obj["actor"] / "zobj.zobj").read_bytes()
        base = len(rom)
        rom += data
        rom += bytes(-len(rom) % ALIGN)

        scan = ObjectScan(data, obj["folder"])
        for name, off in obj["dlists"]:
            scan.add_dlist(off, name)
        for name, off, skel_type in obj["skeletons"]:
            scan.add_skeleton(off, skel_type == "Flex", name)
        staged.append((obj, scan, base))

    rom_path = out / "triforce_objects.z64"
    rom_path.write_bytes(rom)
    sha1 = hashlib.sha1(rom).hexdigest()

    (out / "config.yml").write_text(
        f"{sha1}:\n"
        "  name: TrifoolPercent Triforce objects\n"
        "  path: triforce\n"
        "  config:\n"
        "    gbi: F3DEX2_OoT\n"
        "    sort: OFFSET\n"
        "    logging: INFO\n"
        "    primary_virtual_segment: 0x80\n"
        "    output:\n"
        "      binary: TrifoolPercent-objects.o2r\n"
    )
    for obj, scan, base in staged:
        (out / "triforce" / "objects" / f"{obj['folder']}.yml").write_text(yaml_object(obj, scan, base))
    print(f"{rom_path} sha1={sha1}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
