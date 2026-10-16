#!/usr/bin/env python3
"""Merge Ship of Harkinian resources into one .o2r mod archive.

An .o2r is a zip whose entry names are resource paths (scenes/shared/besitu_scene/besitu_room_0, ...).
Inputs can be folders of resources (Fast64's scene export) or other archives (Torch's output).

Torch also writes a `version` file holding the game version of the ROM it read. Our stand-in ROM is not a real
game version, and libultraship refuses to load an archive with an unknown version, so `version` and
`portVersion` are dropped. libultraship reads manifest.json for the mod's name and credits; this script writes one.

Usage: pack_o2r.py <out.o2r> <resources dir | archive.o2r>...
"""

import json
import sys
import zipfile
from pathlib import Path

MANIFEST = {
    "name": "TrifoolPercent",
    "author": "TrifoolPercent contributors; Triforce% team (original scenes, actors and text)",
    "version": "0.1.0",
    "website": "https://github.com/Bubbagum88/TrifoolPercent",
    "description": "Proof of concept: Triforce% scenes ported to unused SoH debug slots. "
    "Triforce Room in Besitu (0x66); Unicorn Fountain in Test01 (0x65); hex Test Room hub in Syotes (0x68). "
    "The actors, cutscenes and hooks need a Ship of Harkinian build with the TrifoolPercent module.",
    "license": "Triforce% license: credit the Triforce% contributors, no implied endorsement, no warranty.",
}


SKIPPED = {"version", "portVersion", "manifest.json"}


def collect(source: Path) -> dict[str, bytes]:
    if source.is_dir():
        return {p.relative_to(source).as_posix(): p.read_bytes() for p in source.rglob("*") if p.is_file()}
    with zipfile.ZipFile(source) as archive:
        return {name: archive.read(name) for name in archive.namelist() if not name.endswith("/")}


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    out = Path(sys.argv[1])
    resources: dict[str, bytes] = {}
    for source in map(Path, sys.argv[2:]):
        for name, data in collect(source).items():
            if name in SKIPPED:
                continue
            if name in resources and resources[name] != data:
                print(f"error: {name} comes from more than one input with different contents")
                return 1
            resources[name] = data
    if not resources:
        print("no resources found")
        return 1
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in sorted(resources):
            archive.writestr(name, resources[name])
        archive.writestr("manifest.json", json.dumps(MANIFEST, indent=2))
    print(f"{out}: {len(resources)} resources")
    return 0


if __name__ == "__main__":
    sys.exit(main())
