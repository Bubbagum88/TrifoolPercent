<h1 align="center"> ❄️🧊 Tri-FOOL-Percent 🧊❄️ </h1>

A **Ship of Harkinian** mod project based on the legendary Triforce% ACE/TAS run.  
No ACE. No memory corruption. Just pure modding magic.

## Goals
- Recreate the mythical Triforce quest inside SoH
- Port/rebuild key assets (Triforce object, Unicorn Fountain, Beta Fairy, etc.)
- Add custom cutscenes, actors, and quest logic
- Make it fun, cinematic, and community-friendly

## Folder Structure
- **assets/** — models, textures, animations
- **scenes/** — custom and ported scenes
- **actors/** — custom actor definitions
- **cutscenes/** — cutscene scripts
- **scripts/** — quest logic and triggers
- **tools/** — development tools (Fast64, Blender, etc.)
- **docs/** — documentation and [roadmap](docs/roadmap.md)
- **soh/** — the Ship of Harkinian code module (copied into a Shipwright checkout)
- **build/** — final mod files (.o2r)

## Tools
These are the existing tools we use to rebuild Triforce% natively inside Ship of Harkinian. Every link below was checked against a live repository in October 2026.

### Ship of Harkinian
| Tool | What we use it for |
|------|--------------------|
| [Shipwright](https://github.com/HarbourMasters/Shipwright) | The SoH source. Code-side changes (custom actors, quest flags, entrance rerouting) are built against it. Start with its [modding guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/MODDING.md) and [building guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/BUILDING.md). |
| [libultraship](https://github.com/HarbourMasters/libultraship) | The engine layer under SoH. It loads `.o2r`/`.otr` archives, so a mod archive can replace base game resources. Its [scripting guide](https://github.com/HarbourMasters/libultraship/blob/main/docs/SCRIPTING.md) covers the C mod system (`manifest.json`, events) in newer versions; check that the SoH build you target includes it. |

### Building mod archives (`.o2r`)
| Tool | What we use it for |
|------|--------------------|
| [Retro](https://github.com/HarbourMasters/retro) | GUI that packs custom textures, sequences and other files into `.o2r`/`.otr` mods for SoH's `mods` folder. |
| [Torch](https://github.com/HarbourMasters/Torch) | HarbourMasters' asset converter that turns ROM asset definitions into archive resources. |
| [ZAPDTR](https://github.com/HarbourMasters/ZAPDTR) and [OTRExporter](https://github.com/HarbourMasters/OTRExporter) | What SoH uses to build `oot.o2r` from your ROM. Useful for seeing how a scene, object or cutscene is stored as a resource. |

### Models, scenes and actors
| Tool | What we use it for |
|------|--------------------|
| [Fast64](https://github.com/Fast-64/fast64) | Blender plugin that exports OoT models, skeletons, collision, scenes and cutscenes. Triforce% built its Unicorn Fountain, Triforce Room and other scenes with it. HarbourMasters keeps a fork at [HarbourMasters/fast64](https://github.com/HarbourMasters/fast64). |
| [SharpOcarina](https://github.com/Nokaubure/SharpOcarina) | Windows scene and room editor for OoT, for placing actors, exits and cameras. |
| [z64scene](https://github.com/z64me/z64scene) | Cross-platform Zelda 64 scene editor (work in progress). |
| [z64convert](https://github.com/z64me/z64convert) | Converts 3D models into Zelda 64 object files. |
| [Z64Utils](https://github.com/Random06457/Z64Utils) | ROM object viewer for browsing vanilla display lists, textures, skeletons and animations, including unused beta objects. |

### Audio
| Tool | What we use it for |
|------|--------------------|
| [SEQ64](https://github.com/sauraen/seq64) | Sequence (music) editor. SoH imports Seq64 sequences through Retro, as described in its [custom music guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/CUSTOM_MUSIC.md). |
| [z64audio](https://github.com/z64tools/z64audio) | Converts WAV/AIFF audio into N64 sample formats. |

### Reference code
| Project | Why it matters |
|---------|----------------|
| [triforce-percent](https://github.com/triforce-percent/triforce-percent) | The original TASBot Triforce% code, scenes and assets. Breakdown is in [docs/references/triforcepercent](docs/references/triforcepercent/index.md). |
| [zeldaret/oot](https://github.com/zeldaret/oot) | The OoT decompilation. SoH's game code comes from it, so actor and scene structures match. |
| [HackerOoT](https://github.com/HackerN64/HackerOoT) | Decomp-based OoT modding base, a good reference for custom actor and scene patterns. |

## Status
Proof of concept in progress: the Triforce% **Triforce Room**, in the unused Besitu debug-scene slot. See
[docs/design/triforce-room-poc.md](docs/design/triforce-room-poc.md) for how it is built and what has been tested,
and the [roadmap](docs/roadmap.md) for every phase from here to release.

## Porting Checklist
Every mechanic, scene, actor, cutscene and asset from the original Triforce% that needs to land in TrifoolPercent, categorized with checkboxes: [docs/porting-checklist.md](docs/porting-checklist.md).

## How to Use

TrifoolPercent ships as two pieces:

1. **`TrifoolPercent.o2r`** — the asset archive (custom scenes + actor models) that drops into Ship of Harkinian's `mods/` folder.
2. **The `TrifoolPercent` SoH module** — the C code under [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) that must be compiled into `soh.exe`. The `.o2r` loads the scene geometry; the module provides the custom actors, cutscene hooks, text and quest flags.

### Just want to try it (no build)

1. Open the latest green run of the [**build-o2r** workflow](../../actions/workflows/build-o2r.yml) in this repo.
2. Scroll to **Artifacts** and download `TrifoolPercent.o2r`.
3. Drop it into your Ship of Harkinian `mods/` folder.
4. Launch SoH (needs a build that includes the TrifoolPercent module — see Building below) and walk through the blue flame by Link's house in Kokiri Forest.
5. Toggle on/off under **Enhancements → TrifoolPercent** in the SoH menu.

> The GitHub-built `.o2r` alone will load the scenes, but the custom actors and cutscenes only appear with a SoH build that has the module compiled in. Until public module-enabled builds exist, you need to build SoH yourself (see Building the game below).

## Building

Supported hosts: **Linux** (Ubuntu 24.04 is the canonical CI target) or **WSL** on Windows. macOS is unverified. The scripts assume a POSIX shell, `git`, `python3`, `cmake`, `ninja` and a C/C++ toolchain.

### 1. One-time setup

```bash
tools/setup.sh
```

This clones the pinned third-party sources into `third_party/`:

| Dependency | Why it's needed | Pinned to |
|---|---|---|
| [triforce-percent](https://github.com/triforce-percent/triforce-percent) | Source `.blend` files, text and reference zobjs | `12a4bb30…` (last upstream commit, 2023-08-06) |
| [HarbourMasters/fast64](https://github.com/HarbourMasters/fast64) | Blender exporter that produces SoH scene resources | `a1808d81…` (2026-10-11) |
| [HarbourMasters/Shipwright](https://github.com/HarbourMasters/Shipwright) | SoH source, include headers, and the Torch asset tool | `6b4edce4…` on `develop` (2026-10-14) |
| `.venv-blender` | Python 3.13 venv with `bpy==5.2.2` (Blender as a Python module), so Fast64 runs headless | — |

Pins live at the top of [`tools/setup.sh`](tools/setup.sh); edit there to bump.

On Ubuntu you also need Shipwright's build deps before `tools/build_soh.sh`:

```bash
sudo apt-get update
sudo apt-get install -y $(cat third_party/Shipwright/linux-build-deps/apt.txt)
pip install pillow   # for the title-card renderer
```

### 2. Build the mod archive

```bash
tools/build_o2r.sh
```

Writes `build/TrifoolPercent.o2r`. The script:

1. Builds [**Torch**](https://github.com/HarbourMasters/Torch) (HarbourMasters' asset converter) from the Shipwright checkout, OoT support only.
2. Copies this repo's mirrored textures from `assets/scene/<SceneName>/textures/` into the Triforce% clone, then fills any still-missing textures with generated placeholders.
3. Runs **Fast64** headlessly (via `bpy`) on `TriforceRoom.blend` and `UnicornFountain.blend` to export SoH scene/room resources, and renders their `title.ia8` title cards with Pillow.
4. Runs **Torch** on the mirrored Triforce% zobjs under `assets/actor/` to produce the three actor models (Triforce piece, chest, light) as a resource archive.
5. Calls [`tools/pack_o2r.py`](tools/pack_o2r.py) to merge all of the above into one `.o2r` with a `manifest.json` libultraship reads. The Torch-stamped `version` / `portVersion` files are stripped so libultraship doesn't reject the archive for an unknown game version.

Everything stages under `build/stage/`; delete that dir to force a clean rebuild.

### 3. Build the game (optional)

```bash
tools/build_soh.sh
```

Copies [`soh/Enhancements/TrifoolPercent/`](soh/Enhancements/TrifoolPercent) into `third_party/Shipwright/soh/soh/Enhancements/`, then runs Shipwright's normal CMake + Ninja build. Shipwright globs `soh/soh/**`, so there's no CMakeLists to edit. See [Shipwright's BUILDING guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/BUILDING.md) for its own prerequisites. First launch still needs a legal OoT ROM so SoH can extract `oot.o2r` — nothing in this repo ships Nintendo data.

The output is `third_party/Shipwright/build/soh/soh.elf` (or `soh.exe` on Windows); drop `TrifoolPercent.o2r` into its `mods/` folder next to that binary.

### CI / automated builds

[`.github/workflows/build-o2r.yml`](.github/workflows/build-o2r.yml) runs the mod-archive build on every push that touches `scenes/`, `actors/`, `tools/` or the workflow itself, and on manual **Run workflow** dispatch. It is the canonical reference for a clean build: if `tools/build_o2r.sh` ever stops working locally, compare against the latest green run. The resulting `.o2r` is uploaded as a workflow artifact with 30-day retention. The game build (`tools/build_soh.sh`) is not run in CI yet.

## Build Tools Walkthrough

A map of the tools the build touches, in roughly the order it uses them, and what each one actually does for TrifoolPercent.

### `tools/setup.sh` — pinned source fetcher
Shell script. Shallow-clones the three upstream repos at pinned commits and creates `.venv-blender` with `bpy==5.2.2`. Pins (not branch tips) are deliberate: moved submodule branches or upstream refactors won't silently change what a build pulls. Re-running is cheap and idempotent.

### Blender's `bpy` + HarbourMasters Fast64 — scene exporter
[Fast64](https://github.com/HarbourMasters/fast64) is a Blender add-on that exports OoT scenes, rooms, collision, actor placements and cutscenes. We run it **headless** by importing Blender as a Python module (`bpy`) from `.venv-blender`, pointing it at the Triforce% `.blend` files, and loading HM's fork of the exporter as the add-on. The result is a tree of SoH scene/room resources under `build/stage/scene/`. Fast64 reads textures from the folder next to each `.blend`, which is why `build_o2r.sh` copies this repo's `assets/scene/<Name>/textures/` mirror into the clone before each export.

### Pillow — title-card renderer
`scenes/<scene>/make_title_card.py` uses [Pillow](https://pillow.readthedocs.io/) to render each scene's `title.ia8` (the "Besitu" / "Test01" title text SoH shows on entry). Pulled in via `pip install pillow` in CI; `tools/setup.sh` doesn't install it itself, so run it manually or let the workflow do it.

### Torch — asset converter (OoT mode)
[Torch](https://github.com/HarbourMasters/Torch) is the HarbourMasters tool that turns ROM-era asset definitions into libultraship resources. Here it's built from the Shipwright submodule with only `-DBUILD_OOT=ON`, and pointed at the Triforce% zobjs mirrored in `assets/actor/` plus a tiny YAML config generated by [`actors/triforce_actor/make_torch_input.py`](actors/triforce_actor). Output is a resource archive under `build/stage/torch-out/`.

### `tools/pack_o2r.py` — archive merger
A small Python script that walks each input directory or `.o2r`, collects every entry as `{path: bytes}`, and writes one zip. It drops `version` / `portVersion` (Torch stamps the ROM version it read, which isn't a real SoH game version) and writes a fresh `manifest.json` — libultraship reads that for the mod's name, author, website and description shown in SoH's Mods list. `.o2r` is just a zip; you can `unzip -l build/TrifoolPercent.o2r` to see what shipped.

### Shipwright + CMake/Ninja — the game build
[`tools/build_soh.sh`](tools/build_soh.sh) copies the module source into the Shipwright checkout and runs its normal Release build (`cmake -G Ninja … && cmake --build`). The module itself is plain C using Shipwright's hook/event system — custom actors register in [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) and are wired into scenes by the `.o2r`.

### GitHub Actions — reproducible builds
The `build-o2r` workflow runs the whole pipeline on `ubuntu-24.04`: checkout, Python 3.13, Pillow, `tools/setup.sh`, Shipwright's apt deps, `GenerateSohOtr` (needed to seed Torch's include tables), `tools/build_o2r.sh`, then `actions/upload-artifact` for the `.o2r`. `concurrency` cancels in-flight runs on the same ref so pushes don't pile up. If you ever need to debug a build, click into a run → the first red step name says exactly where it broke.

### How we built and modify the mod
- **Scenes** live as `.blend` files upstream in `third_party/triforce-percent/scene/`; iterate on them in Blender, re-run `tools/build_o2r.sh`, drop the new `.o2r` in SoH's `mods/` folder.
- **Textures** that need to ship with the mod go in `assets/scene/<Scene>/textures/` and are picked up on the next build.
- **Actor models** are the Triforce% zobjs under `assets/actor/`; swap the bytes or re-export via z64convert/Fast64, then rebuild.
- **Gameplay changes** (new actors, cutscene hooks, quest flags, menu toggles) live in [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) and ride along with `tools/build_soh.sh`.
- **Reference code** for original behavior sits in `third_party/triforce-percent/` after setup, and the breakdown at [`research/triforce-percent-code-breakdown.md`](research/triforce-percent-code-breakdown.md) maps the pieces we're porting.

## Inspiration
- Original Triforce% project
- OoT beta content
- Ship of Harkinian modding community

## Acknowledgements

TrifoolPercent would not exist without the pioneering work of **The Triforce% Team (2019–2022)**. Their original ACE/TAS showcase, source code and custom assets are the foundation this project ports onto Ship of Harkinian.

- Upstream repository: [triforce-percent/triforce-percent](https://github.com/triforce-percent/triforce-percent)
- Upstream site: [gettriforce.link](https://gettriforce.link)
- Live run: [TASBot's Triforce% at AGDQ 2023](https://www.youtube.com/watch?v=2x_pqyrf9lA)
- A vendored copy of their LICENSE and README lives under [`upstream/`](upstream/).

Vendored source trees from the upstream repo in this project (all authored by the Triforce% Team, no Nintendo content):

- [`actors/`](actors/) — custom and patched actor sources
- [`scenes/`](scenes/) — custom scenes and vanilla scene patches
- [`assets/anim`](assets/anim/), [`assets/model`](assets/model/), [`assets/music`](assets/music/), [`assets/textures`](assets/textures/), [`assets/voice`](assets/voice/), [`assets/text`](assets/text/) — animations, models, music, textures, voice clips and message text
- [`scripts/statics/`](scripts/statics/) — engine-glue statics (quest state, scene wiring, ocarina, audio, interface, messages)

Per the upstream license, members of the Triforce% Team **must be credited in the TrifoolPercent in-game Staff Roll** when the mod ships. Builds of TrifoolPercent do not and will not redistribute ROM data or any assets copyrighted by Nintendo.

## Special Thanks

Huge thanks to the broader **Ocarina of Time modding and preservation community** — decades of their reverse engineering, cataloguing and documentation of OoT's unused content is what makes a project like this feasible for one person. In particular, these references saved an enormous amount of work:

- [**TCRF — Unused Actors & Objects**](https://tcrf.net/The_Legend_of_Zelda:_Ocarina_of_Time/Unused_Actors_%26_Objects) — every unused actor and object listed with its ID/address. Goated.
- [**TCRF — Unused Content in Used Actors & Objects**](https://tcrf.net/The_Legend_of_Zelda:_Ocarina_of_Time/Unused_Content_in_Used_Actors_%26_Objects) — same, for content hiding inside shipped actors and objects.
- [**Zelda Wiki — Unused Content (OoT)**](https://zelda.fandom.com/wiki/Unused_Content) — overview, screenshots and historical context for beta/cut material.
- [**jaytheham.com — OoT Beta Content**](https://www.jaytheham.com/zcw/Ocarina_of_Time_Beta_Content.html) — ready-to-use access to beta assets without hunting addresses. Massive for iterating on this mod.

And of course the people and projects behind the tooling TrifoolPercent is built on: **HarbourMasters** (Ship of Harkinian, Fast64 fork, Torch, Retro), the **OoT decompilation team** at [zeldaret/oot](https://github.com/zeldaret/oot), **HackerOoT**, **Fast64**, **z64me**, **Sauraen** (SEQ64), **Random06457** (Z64Utils), and everyone contributing to the SoH modding community.

Contributions welcome!
