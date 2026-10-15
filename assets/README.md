# assets/

Published source assets from the Triforce% repo, mirrored here so the TrifoolPercent build does not depend on
`third_party/triforce-percent/` being reachable at build time (its repo could move, go private, or vanish).

Everything under this directory was imported from Triforce% at a point in time. Credit stays with the
Triforce% team (see [`CREDITS.md`](../CREDITS.md) and the Triforce% license their repo ships).

## What's in here

| Path | What it holds | Who reads it |
|---|---|---|
| `actor/<Actor>/zobj.zobj` | Compiled N64 object files for the Triforce piece, chest and light actors. The models and animations. | `actors/triforce_actor/make_torch_input.py`, which walks each zobj and emits YAML for Torch to convert. |
| `scene/TriforceRoom/textures/*.png` | Published textures the Triforce Room `.blend` references. | Blender's Fast64 reads these next to the `.blend`, so the build copies them into `third_party/triforce-percent/scene/TriforceRoom/textures/` before export. |
| `scene/UnicornFountain/textures/*.png` | Published textures the Unicorn Fountain `.blend` references. | Same as above, copied in next to the fountain's `.blend`. |
| `scene/UnicornFountain/title.ia8.png` | The fountain's place-name title card as a PNG. | `scenes/unicorn_fountain/make_title_card.py` reads it directly from here, converts it to IA8, and writes the resource. |
| `music/`, `voice/`, `text/`, `anim/`, `model/`, `textures/` | Other Triforce% source assets (sequences, voice clips, custom animations, Sages' Charm model, etc.) imported earlier for later phases. | Not consumed yet; wired up as each phase lands. |

## What's not in here

- The scene `.blend` files. Too big to mirror sensibly, and iterated upstream in the Triforce% repo. The
  build still clones Triforce% (shallow) via `tools/setup.sh` for them.
- Textures that Triforce% never published (ripped from the ROM at build time). Those are filled in at
  build time by `scenes/*/make_placeholder_textures.py`.
- The fairy's zobj. Never published; see the Beta Great Fairy actor source for the resulting porting gap.

## Updating

Mirrors are snapshots. If Triforce% publishes new or updated assets we want, copy them into the matching
path here, keeping the same layout. Record the Triforce% commit the snapshot came from in the commit
message so the provenance is traceable.
