<h1 align="center"> ❄️🧊 Tri-FOOL-Percent 🧊❄️ </h1>

<p align="center">
A native <strong>Ship of Harkinian</strong> rebuild of the Triforce% ACE/TAS run.<br>
No ACE. No memory corruption. Mod code.
</p>

<p align="center">
  <a href="../../actions/workflows/build-o2r.yml"><img alt="build-o2r" src="../../actions/workflows/build-o2r.yml/badge.svg"></a>
  <a href="docs/roadmap.md"><img alt="roadmap" src="https://img.shields.io/badge/roadmap-live-blue"></a>
  <a href="CONTRIBUTING.md"><img alt="PRs welcome" src="https://img.shields.io/badge/PRs-welcome-brightgreen"></a>
</p>

---

## About

TrifoolPercent rebuilds the Triforce% quest — the Chamber of Sages, Unicorn Fountain, Triforce Room, the Three Keys and the staff roll — as a Ship of Harkinian mod. Upstream Triforce% ran as an ACE payload squeezed into a corrupted save on real hardware. We rebuild the same beats as real SoH content: scenes exported with Fast64, actors registered through `ActorDB`, quest logic hung off `GameInteractor` hooks, text through `CustomMessageManager`, entrances rewired without touching the entrance table.

Done means a player with a legal OoT ROM can install the mod, start a new file, and play the quest from the Lost Woods to the staff roll — no debug warps, no corruption scaffolding, saves still compatible with vanilla SoH.

Two deliverables:

- `TrifoolPercent.o2r` — scenes, rooms, objects, textures, dropped into SoH's `mods/` folder.
- The `soh/Enhancements/TrifoolPercent/` module — compiled into a Shipwright build, gated behind one CVar toggle under **Enhancements → TrifoolPercent**.

No Nintendo ROM data is redistributed. SoH extracts `oot.o2r` from your own OoT ROM on first launch.

## Quick start

1. Download `TrifoolPercent.o2r` from the [build-o2r workflow artifacts](../../actions/workflows/build-o2r.yml).
2. Drop it in SoH's `mods/` folder.
3. Launch a SoH build with the [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) module compiled in — see [BUILDING.md](docs/BUILDING.md).
4. Walk through the blue flame by Link's house in Kokiri Forest.

## What works today

- [x] Triforce Room PoC in the unused Besitu debug slot
- [x] Three Triforce actors (`Piece`, `Chest`, `Light`) via `ActorDB`
- [x] Kokiri Forest onboarding beat, Lost Woods path, Beta Kokiri NPC
- [x] Butterfly trade on the `ITEM_BIG_POE` slot
- [x] Unicorn Fountain scene + Overture dialogue in the Test01 slot
- [x] Overworld Fountain entrance via Lost Woods + Door of Time adult hook
- [x] Entrance rewiring to scene slots instead of patching `gEntranceTable`
- [ ] Long of Time age-flip consumer, Medallion-on-C, staff-roll chain, Chamber of Sages — [parity](docs/references/triforcepercent-parity.md), [roadmap](docs/roadmap.md), [porting checklist](docs/porting-checklist.md)

## Why rebuild it on SoH

Upstream had to fight the N64. We don't. The engine already gives us:

- a resource system that loads `.o2r` scenes and objects without ROM patches;
- `ActorDB` and `GameInteractor` for custom actors and hooks;
- `CustomMessageManager` for new text;
- Bunny Hood speed + jump, Ocarina time travel without the Master Sword, debug warps, a save editor;
- widescreen, high-FPS internal timing, modern input, and a menu to toggle the mod cleanly.

Where upstream patches a routine, we hook the equivalent SoH event. Where upstream relies on ACE scaffolding, we delete it. The parity table in [`docs/references/triforcepercent-parity.md`](docs/references/triforcepercent-parity.md) tracks every patch: matched, done differently, or still missing.

## Documentation

| Doc | What's in it |
|-----|--------------|
| [docs/BUILDING.md](docs/BUILDING.md) | Build the `.o2r` and the SoH module. |
| [docs/BUILD_TOOLS.md](docs/BUILD_TOOLS.md) | Tool-by-tool pipeline walkthrough. |
| [docs/TOOLS.md](docs/TOOLS.md) | OoT / SoH ecosystem (editors, converters, references). |
| [docs/roadmap.md](docs/roadmap.md) | Phases from PoC to release. |
| [docs/porting-checklist.md](docs/porting-checklist.md) | Every mechanic / scene / actor / cutscene to port. |
| [docs/references/triforcepercent-parity.md](docs/references/triforcepercent-parity.md) | Patch-by-patch Triforce% ↔ TrifoolPercent status. |
| [docs/devlog.md](docs/devlog.md) | What was tried, what broke, why decisions landed where they did. |
| [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md) | Triforce% Team, OoT community, references. |
| [CONTRIBUTING.md](CONTRIBUTING.md) | PR guide and review checklist. |
| [CREDITS.md](CREDITS.md) | Per-contributor roll. |

## Community

- Bugs / feature requests → [issue templates](.github/ISSUE_TEMPLATE)
- General SoH-modding chat → [Shipwright discussions](https://github.com/HarbourMasters/Shipwright/discussions)
- Original Triforce% source → [triforce-percent/triforce-percent](https://github.com/triforce-percent/triforce-percent)

## Folder structure

<details>
<summary>Click to expand</summary>

- `assets/` — models, textures, animations
- `scenes/` — custom and ported scenes
- `actors/` — custom actor sources
- `cutscenes/` — cutscene scripts
- `scripts/` — quest logic and triggers
- `tools/` — `setup.sh`, `build_o2r.sh`, `build_soh.sh`, `pack_o2r.py`
- `soh/` — SoH module, copied into Shipwright at build time
- `build/` — final `.o2r`
- `docs/` — documentation
- `screenshots/` — in-game captures
- `upstream/` — vendored Triforce% LICENSE + README
- `.github/workflows/` — CI
</details>

## Acknowledgements

Built on the work of the **Triforce% Team** (the quest, the scenes, the assets), **HarbourMasters** (Ship of Harkinian), the **zeldaret/oot** decomp team, and the OoT modding and preservation community (TCRF, Zelda Wiki, [The Spriters Resource](https://models.spriters-resource.com/nintendo_64/thelegendofzeldaocarinaoftime/)). Per the upstream license, the Triforce% Team is credited in the in-game staff roll when the mod ships. Full roll in [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md).

## Contributing

PRs welcome — see [CONTRIBUTING.md](CONTRIBUTING.md). Start with [BUILDING.md](docs/BUILDING.md) and pick an open row from the [porting checklist](docs/porting-checklist.md) or the [parity table](docs/references/triforcepercent-parity.md).
