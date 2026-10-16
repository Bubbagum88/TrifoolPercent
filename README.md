<h1 align="center"> ❄️🧊 Tri-FOOL-Percent 🧊❄️ </h1>

<p align="center">
A <strong>Ship of Harkinian</strong> mod project based on the legendary Triforce% ACE/TAS run.<br>
No ACE. No memory corruption. Just pure modding magic.
</p>

---

## Goals
- Recreate the mythical Triforce quest inside SoH
- Port/rebuild key assets (Triforce object, Unicorn Fountain, Beta Fairy, etc.)
- Add custom cutscenes, actors, and quest logic
- Make it fun, cinematic, and community-friendly

## Status
Proof of concept in progress: the Triforce% **Triforce Room**, in the unused Besitu debug-scene slot. See
[docs/design/triforce-room-poc.md](docs/design/triforce-room-poc.md) for how it is built and what has been tested,
and the [roadmap](docs/roadmap.md) for every phase from here to release.

## Documentation

| Doc | What's in it |
|-----|--------------|
| [docs/BUILDING.md](docs/BUILDING.md) | How to install, build the `.o2r`, build the game, and use the result. |
| [docs/BUILD_TOOLS.md](docs/BUILD_TOOLS.md) | Walkthrough of every tool in the build pipeline, in the order it runs. |
| [docs/TOOLS.md](docs/TOOLS.md) | The wider OoT / Ship of Harkinian tooling ecosystem (editors, exporters, audio tools, references). |
| [docs/roadmap.md](docs/roadmap.md) | Phased plan from PoC to release. |
| [docs/porting-checklist.md](docs/porting-checklist.md) | Every mechanic, scene, actor, cutscene and asset to port, with checkboxes. |
| [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md) | The Triforce% Team, the OoT community, and the references and tooling we build on. |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to send a PR, what belongs where, and the review checklist. |
| [CREDITS.md](CREDITS.md) | Per-contributor roll for this repo. |

## Quick start

Grab the latest `TrifoolPercent.o2r` from the [**build-o2r** workflow artifacts](../../actions/workflows/build-o2r.yml), drop it in your Ship of Harkinian `mods/` folder, launch SoH (with a build that includes this repo's [`soh/Enhancements/TrifoolPercent`](soh/Enhancements/TrifoolPercent) module — see [BUILDING.md](docs/BUILDING.md)), and walk through the blue flame by Link's house in Kokiri Forest. Toggle under **Enhancements → TrifoolPercent**.

## Folder Structure
- **assets/** — models, textures, animations
- **scenes/** — custom and ported scenes
- **actors/** — custom actor definitions
- **cutscenes/** — cutscene scripts
- **scripts/** — quest logic and triggers
- **tools/** — build scripts (setup, build_o2r, build_soh, pack_o2r)
- **soh/** — the Ship of Harkinian code module (copied into a Shipwright checkout)
- **build/** — final mod files (`.o2r`)
- **docs/** — documentation (building, build-tools walkthrough, roadmap, acknowledgements, references)
- **upstream/** — vendored LICENSE and README from the Triforce% project

## Acknowledgements

TrifoolPercent stands on the work of **The Triforce% Team**, the **Ocarina of Time modding and preservation community** (TCRF, Zelda Wiki, jaytheham.com and many more), **HarbourMasters** and the **zeldaret/oot decomp team**. See [docs/ACKNOWLEDGEMENTS.md](docs/ACKNOWLEDGEMENTS.md) for the full roll.

Builds of TrifoolPercent do not and will not redistribute ROM data or any assets copyrighted by Nintendo.

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md).
