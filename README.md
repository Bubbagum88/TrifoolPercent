<h1 align="center"> ❄️🧊 Tri-FOOL-Percent 🧊❄️ </h1>

<p align="center">
A <strong>Ship of Harkinian</strong> mod project based on the legendary Triforce% ACE/TAS run.<br>
No ACE. No memory corruption. Just pure modding magic.
</p>

<p align="center">
  <a href="../../actions/workflows/build-o2r.yml"><img alt="build-o2r" src="../../actions/workflows/build-o2r.yml/badge.svg"></a>
  <a href="docs/roadmap.md"><img alt="roadmap" src="https://img.shields.io/badge/roadmap-live-blue"></a>
  <a href="CONTRIBUTING.md"><img alt="contributions welcome" src="https://img.shields.io/badge/PRs-welcome-brightgreen"></a>
</p>

---

## Goals
- Recreate the mythical Triforce quest inside SoH
- Port/rebuild key assets (Triforce object, Unicorn Fountain, Beta Fairy, etc.)
- Add custom cutscenes, actors, and quest logic
- Make it fun, cinematic, and community-friendly

## Status
Proof of concept in progress: the Triforce% **Triforce Room**, in the unused Besitu debug-scene slot, plus the Kokiri onboarding beat and the Lost-Woods hub.

- How the PoC is built and tested: [docs/design/triforce-room-poc.md](docs/design/triforce-room-poc.md)
- Full phased plan from PoC to release: [docs/roadmap.md](docs/roadmap.md)
- Every mechanic, scene, actor, cutscene and asset to port, with checkboxes: [docs/porting-checklist.md](docs/porting-checklist.md)

## Quick start

1. Grab the latest `TrifoolPercent.o2r` from the [**build-o2r** workflow artifacts](../../actions/workflows/build-o2r.yml).
2. Drop it in your Ship of Harkinian `mods/` folder.
3. Launch SoH (needs a build that includes this repo's [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) module — see [BUILDING.md](docs/BUILDING.md)).
4. Walk through the blue flame by Link's house in Kokiri Forest. Toggle under **Enhancements → TrifoolPercent**.

Want to build it yourself? See [docs/BUILDING.md](docs/BUILDING.md). Want to understand the pipeline? See [docs/BUILD_TOOLS.md](docs/BUILD_TOOLS.md).

## Documentation

| Doc | What's in it |
|-----|--------------|
| [docs/BUILDING.md](docs/BUILDING.md) | Install, build the `.o2r`, build the game, use the result. |
| [docs/BUILD_TOOLS.md](docs/BUILD_TOOLS.md) | Tool-by-tool walkthrough of the build pipeline. |
| [docs/TOOLS.md](docs/TOOLS.md) | Wider OoT / Ship of Harkinian ecosystem (editors, converters, audio, references). |
| [docs/roadmap.md](docs/roadmap.md) | Phased plan from PoC to release. |
| [docs/porting-checklist.md](docs/porting-checklist.md) | What's left to port, with checkboxes. |
| [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md) | Triforce% Team, OoT community, references, tooling. |
| [CONTRIBUTING.md](CONTRIBUTING.md) | PR guide, what belongs where, review checklist. |
| [CREDITS.md](CREDITS.md) | Per-contributor roll for this repo. |
| [docs/ABOUT_SIDEBAR.md](docs/ABOUT_SIDEBAR.md) | Copy-paste config for the GitHub "About" sidebar and discovery topics. |

## Folder Structure

<details>
<summary>Click to expand</summary>

- **assets/** — models, textures, animations
- **scenes/** — custom and ported scenes
- **actors/** — custom actor definitions
- **cutscenes/** — cutscene scripts
- **scripts/** — quest logic and triggers
- **tools/** — build scripts (`setup.sh`, `build_o2r.sh`, `build_soh.sh`, `pack_o2r.py`)
- **soh/** — Ship of Harkinian code module (copied into a Shipwright checkout at build time)
- **build/** — final mod files (`.o2r`)
- **docs/** — documentation (building, build-tools walkthrough, roadmap, acknowledgements, references)
- **screenshots/** — in-game captures used in docs and dev-logs
- **upstream/** — vendored LICENSE and README from the Triforce% project
- **.github/workflows/** — CI (`build-o2r.yml`)
</details>

## Acknowledgements

TrifoolPercent stands on the work of **The Triforce% Team**, the **Ocarina of Time modding and preservation community** (TCRF, Zelda Wiki, [jaytheham.com](https://www.jaytheham.com/zcw/Ocarina_of_Time_Beta_Content.html) and many more), **HarbourMasters** and the **zeldaret/oot decomp team**. Full roll in [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md).

Builds of TrifoolPercent do not and will not redistribute ROM data or any assets copyrighted by Nintendo.

## Contributing

PRs welcome — see [CONTRIBUTING.md](CONTRIBUTING.md). First-time contributors: start with [docs/BUILDING.md](docs/BUILDING.md) and skim the [porting checklist](docs/porting-checklist.md) for an open beat to pick up.
