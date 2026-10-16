# Converting the Unicorn Fountain cutscene

**Status, 2026-10-16:** Not done. The fountain scene ships without its cutscene, and
`TfBetaGreatFairy` auto-advances from state 0 after `AUTO_RISE_FRAMES=60` as a stand-in.

This document captures what the conversion actually requires, since it is substantially
bigger than the Triforce Room's was.

## Why this isn't a Fast64 export

The Triforce Room cutscene lives in Triforce%'s shipped
`scene/TriforceRoom/TriforceRoom_scene.c` as a `CutsceneData` C array; our build copies
it into `soh/Enhancements/TrifoolPercent/TriforceRoomCs.c` with a few vanilla-SoH
patches (see that file's header). The fountain has no equivalent `.c` upstream:

- `scene/UnicornFountain/` ships only `UnicornFountain.blend`,
  `UnicornFountain_scene.out.ld`, `title.ia8.png`, textures and `conf.txt`.
- `scene/Makefile` has the fountain build **commented out** — Triforce% never
  shipped the fountain in a release, and no `.c` was ever generated.
- The `.ld` reports a single symbol: `UnicornFountain_scene_header00_cutscene = 0x20012e0`,
  which is an offset *inside* the compiled scene binary, not a shippable C array.

The cutscene *is* present inside `UnicornFountain.blend` as a vanilla-Fast64
`Cutscene.UnicornFountain_scene_header00_cutscene` object (seen in strings output:
`zcamedit`, `OOT_PT_export_cutscene`, `textbox`, `textboxType` properties).

## Why HM Fast64's SoH export doesn't pick it up

HM Fast64's `export_hm64_scene` (`third_party/addons/fast64/fast64_internal/hm64/z64/scene.py`)
writes the scene header's cutscene command (`_CMD["CUTSCENES"] = 0x17`) only when a
`cutscene_path` is already present in the **reference archive** it's exporting against —
see `_base_header_cutscene_paths` at line 1587. It does not serialize a Blender
`Cutscene` object into bytes. The vanilla upstream Fast64 had a cutscene-to-C writer;
the HM fork removed it.

So even though the fountain's `.blend` has cutscene data, our existing SoH-mode export
drops it on the floor.

## The three realistic paths

### (a) Run upstream vanilla Fast64 once, convert the resulting `.c`

Install upstream Fast64 (not the HM fork) in a scratch Blender, run its "Export
Cutscene" operator on `Cutscene.UnicornFountain_scene_header00_cutscene`, get a `.c`
with `CutsceneData gUnicornFountainCs[]`, then hand-port it the way we did
`TriforceRoomCs.c`: port vanilla `CS_*` macros to SoH ones (they are mostly the same),
fix raw-float camera angles, drop any terminator that would warp into a Triforce%
scene we don't ship.

Smallest total work. The `.c` is a one-time export; after that it's a static file in
this repo.

### (b) Build a Blender-side extractor in this repo

Add a `scenes/unicorn_fountain/export_cutscene.py` that uses `bpy` to walk the
`Cutscene` object's `csLists` collection, read each `csCmdType` entry (textbox,
camera command, lighting, misc, etc.), and emit the SoH `CS_*` macros directly.
About 300–500 lines, has to handle every command type the fountain actually uses
(textbox list, action lists, camera eye and at splines, lighting, misc, maybe scene
transition at the end).

Reusable (same script would convert the Ending cutscene later), but a real
project, not a 10-minute script.

### (c) Hand-write a stub cutscene

Author a short `UnicornFountainCs.c` with just enough commands to make the fairy
visible and advance her state machine:
`CS_BEGIN_CUTSCENE(2, N)`, one `CS_PLAYER_CUE_LIST`, one `CS_NPC_ACTION_LIST` driving
the fairy's `csAction` through its states, one `CS_TEXT_LIST` with the Overture
pickup message, and `CS_END`. Loses the camera choreography and the dialogue
flow, but unblocks in-game testing.

## Recommendation

**(a)** when someone with a Blender install can do the one-time export. We land in
the same shape as `TriforceRoomCs.c` and the fairy's state machine switches from
timer-driven to cutscene-driven. Until then, the fairy's `AUTO_RISE_FRAMES` fallback
in `z_tf_beta_great_fairy.c` keeps the actor demoable without the cutscene.

## Key references

- `third_party/triforce-percent/scene/UnicornFountain/UnicornFountain.blend` — the
  only place the cutscene actually exists.
- `third_party/triforce-percent/scene/UnicornFountain/UnicornFountain_scene.out.ld` —
  the symbol pointing at offset 0x12e0 inside the never-shipped scene binary.
- `third_party/triforce-percent/scene/Makefile` — the fountain build commented out.
- `third_party/addons/fast64/fast64_internal/hm64/z64/scene.py:1587` —
  `_base_header_cutscene_paths`, which proves the HM exporter only preserves
  existing cutscene refs, never generates them.
- `soh/Enhancements/TrifoolPercent/TriforceRoomCs.c` — the shape we're aiming for.
- `soh/Enhancements/TrifoolPercent/actors/z_tf_beta_great_fairy.c` — the
  `AUTO_RISE_FRAMES` fallback that stands in until the cutscene is wired up.
