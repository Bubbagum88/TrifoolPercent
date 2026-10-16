# Contributing to TrifoolPercent

Pull requests are welcome! By submitting one, you agree to be credited in [CREDITS.md](CREDITS.md) under your preferred name or handle.

## Before you start

- Read [docs/BUILDING.md](docs/BUILDING.md) and get a local build going. If the build doesn't work on your box, please open an issue — that's a bug.
- Skim [docs/BUILD_TOOLS.md](docs/BUILD_TOOLS.md) so you know which stage of the pipeline a change belongs in.
- Check the [roadmap](docs/roadmap.md) and the [porting checklist](docs/porting-checklist.md) to see what's already planned.
- Keep each PR focused — one scene, one actor, one build-script fix at a time is much easier to review than a mixed change.

## What belongs where

- **Scenes / rooms** → `scenes/<Scene>/` for the Fast64/Blender exporter wiring; the source `.blend` lives upstream in `third_party/triforce-percent/scene/`.
- **Shipped textures** → `assets/scene/<Scene>/textures/`.
- **Actor models (zobjs)** → `assets/actor/`.
- **Gameplay logic** (custom actors, cutscene hooks, quest flags, menu toggles) → [`soh/Enhancements/TrifoolPercent/`](soh/Enhancements/TrifoolPercent).
- **Build pipeline changes** → [`tools/`](tools/) and [`.github/workflows/`](.github/workflows/).
- **Docs** → `docs/` (and a short link from the README if it's something new users should see).

## Pull request checklist

- `tools/build_o2r.sh` builds cleanly locally.
- The CI `build-o2r` workflow is green on your branch.
- If you touched the SoH module, `tools/build_soh.sh` builds cleanly.
- You added yourself to [CREDITS.md](CREDITS.md) (or confirm in the PR you're happy with the existing entry).
- Nintendo-copyrighted assets (ROM data, vanilla textures, music, etc.) are **not** in your diff. Everything we ship either comes from the Triforce% upstream (which is already covered) or is original.

## Licensing

See [upstream/TRIFORCE-PERCENT-LICENSE.txt](upstream/TRIFORCE-PERCENT-LICENSE.txt) for the terms covering the vendored Triforce% source. A repo-level LICENSE for TrifoolPercent's own additions is still TBD — until one lands, contributions are made under the understanding that they'll be relicensed under whatever permissive license the project ends up picking (MIT, Apache-2.0, or similar).

## Questions?

Open a draft PR or a GitHub issue. There's no bikeshed too small — if the docs are unclear, that's a bug too.
