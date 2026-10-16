# Building TrifoolPercent

> See also: [BUILD_TOOLS.md](BUILD_TOOLS.md) (what each tool does) · [TOOLS.md](TOOLS.md) (wider ecosystem) · [CONTRIBUTING.md](../CONTRIBUTING.md)

**Contents**
- [What it ships](#what-it-ships)
- [Just want to try it (no build)](#just-want-to-try-it-no-build)
- [Prerequisites](#prerequisites)
- [1. One-time setup](#1-one-time-setup)
- [2. Build the mod archive](#2-build-the-mod-archive)
- [3. Build the game (optional)](#3-build-the-game-optional)
- [CI / automated builds](#ci--automated-builds)
- [Troubleshooting](#troubleshooting)

## What it ships

TrifoolPercent ships as two pieces:

1. **`TrifoolPercent.o2r`** — the asset archive (custom scenes + actor models) that drops into Ship of Harkinian's `mods/` folder.
2. **The `TrifoolPercent` SoH module** — the C code under [`soh/Enhancements/TrifoolPercent`](../soh/Enhancements/TrifoolPercent) that must be compiled into `soh.exe`. The `.o2r` loads the scene geometry; the module provides the custom actors, cutscene hooks, text and quest flags.

## Just want to try it (no build)

1. Open the latest green run of the [**build-o2r** workflow](../../../actions/workflows/build-o2r.yml) in this repo.
2. Scroll to **Artifacts** and download `TrifoolPercent.o2r`.
3. Drop it into your Ship of Harkinian `mods/` folder.
4. Launch SoH (needs a build that includes the TrifoolPercent module — see below) and walk through the blue flame by Link's house in Kokiri Forest.
5. Toggle on/off under **Enhancements → TrifoolPercent** in the SoH menu.

> The GitHub-built `.o2r` alone will load the scenes, but the custom actors and cutscenes only appear with a SoH build that has the module compiled in. Until public module-enabled builds exist, you need to build SoH yourself (step 3 below).

## Prerequisites

Supported hosts: **Linux** (Ubuntu 24.04 is the canonical CI target) or **WSL** on Windows. macOS is unverified. The scripts assume a POSIX shell, `git`, `python3`, `cmake`, `ninja` and a C/C++ toolchain.

## 1. One-time setup

```bash
tools/setup.sh
```

Clones the pinned third-party sources into `third_party/`:

| Dependency | Why it's needed | Pinned to |
|---|---|---|
| [triforce-percent](https://github.com/triforce-percent/triforce-percent) | Source `.blend` files, text and reference zobjs | `12a4bb30…` (last upstream commit, 2023-08-06) |
| [HarbourMasters/fast64](https://github.com/HarbourMasters/fast64) | Blender exporter that produces SoH scene resources | `a1808d81…` (2026-10-11) |
| [HarbourMasters/Shipwright](https://github.com/HarbourMasters/Shipwright) | SoH source, include headers, and the Torch asset tool | `6b4edce4…` on `develop` (2026-10-14) |
| `.venv-blender` | Python 3.13 venv with `bpy==5.2.2` (Blender as a Python module), so Fast64 runs headless | — |

Pins live at the top of [`tools/setup.sh`](../tools/setup.sh); edit there to bump.

On Ubuntu you also need Shipwright's build deps before step 3:

```bash
sudo apt-get update
sudo apt-get install -y $(cat third_party/Shipwright/linux-build-deps/apt.txt)
pip install pillow   # for the title-card renderer
```

## 2. Build the mod archive

```bash
tools/build_o2r.sh
```

Writes `build/TrifoolPercent.o2r`. The script:

1. Builds [**Torch**](https://github.com/HarbourMasters/Torch) (HarbourMasters' asset converter) from the Shipwright checkout, OoT support only.
2. Copies this repo's mirrored textures from `assets/scene/<SceneName>/textures/` into the Triforce% clone, then fills any still-missing textures with generated placeholders.
3. Runs **Fast64** headlessly (via `bpy`) on `TriforceRoom.blend` and `UnicornFountain.blend` to export SoH scene/room resources, and renders their `title.ia8` title cards with Pillow.
4. Runs **Torch** on the mirrored Triforce% zobjs under `assets/actor/` to produce the three actor models (Triforce piece, chest, light) as a resource archive.
5. Calls [`tools/pack_o2r.py`](../tools/pack_o2r.py) to merge all of the above into one `.o2r` with a `manifest.json` libultraship reads. The Torch-stamped `version` / `portVersion` files are stripped so libultraship doesn't reject the archive for an unknown game version.

Everything stages under `build/stage/`; delete that dir to force a clean rebuild.

## 3. Build the game (optional)

```bash
tools/build_soh.sh
```

Copies [`soh/Enhancements/TrifoolPercent/`](../soh/Enhancements/TrifoolPercent) into `third_party/Shipwright/soh/soh/Enhancements/`, then runs Shipwright's normal CMake + Ninja build. Shipwright globs `soh/soh/**`, so there's no CMakeLists to edit. See [Shipwright's BUILDING guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/BUILDING.md) for its own prerequisites. First launch still needs a legal OoT ROM so SoH can extract `oot.o2r` — nothing in this repo ships Nintendo data.

The output is `third_party/Shipwright/build/soh/soh.elf` (or `soh.exe` on Windows); drop `TrifoolPercent.o2r` into its `mods/` folder next to that binary.

## CI / automated builds

[`.github/workflows/build-o2r.yml`](../.github/workflows/build-o2r.yml) runs the mod-archive build on every push that touches `scenes/`, `actors/`, `tools/` or the workflow itself, and on manual **Run workflow** dispatch. It is the canonical reference for a clean build: if `tools/build_o2r.sh` ever stops working locally, compare against the latest green run. The resulting `.o2r` is uploaded as a workflow artifact with 30-day retention. The game build (`tools/build_soh.sh`) is not run in CI yet.

See also: [BUILD_TOOLS.md](BUILD_TOOLS.md) for a tool-by-tool walkthrough of what each piece of the pipeline does.

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `bpy==5.2.2` wheel won't install | Not on Python 3.13 (the pinned bpy wheel is Python-version-specific) | Install Python 3.13 and recreate `.venv-blender`. |
| `tools/build_o2r.sh` fails inside Fast64 with `has faces assigned to a material which is not a F3D material` | A scene mesh has a plain Blender material instead of an F3D material | Open the `.blend` in Blender, re-slot the mesh's material onto an F3D-aware one (see the `TriforceRoom.blend` → `TestRoom.blend` donor pattern). |
| `Torch` build fails on first run | Missing C/C++ toolchain or wrong CMake version | Install the Shipwright `linux-build-deps` listed in step 1. |
| `pack_o2r.py` writes a `.o2r` that SoH refuses with "unknown version" | A `version` or `portVersion` entry slipped past the archive merger | Delete `build/stage/` and rebuild from a clean tree. |
| `tools/build_soh.sh` fails before CMake runs | `third_party/Shipwright/` not checked out | Run `tools/setup.sh` again. |

If something else goes wrong, open an issue with the exact command, the terminal output, and whether CI reproduces it. The `build-o2r` workflow is the canonical reference — if a step is green there and red for you, the delta is almost always your environment.
