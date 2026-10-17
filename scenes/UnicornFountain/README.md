# UnicornFountain

Blender assets for the Triforce% Unicorn Fountain, ported as a native SoH scene.
This folder is the artist-side input (the `.blend` plus textures and a title
card); the Fast64 export tooling and runtime glue live elsewhere.

Scene slot: `SCENE_TEST01` (unused OoT debug slot 0x65 — "115:Test Map 1" in
the SoH map-select). Reached in-game via the hex Test Room's entrance 4
(`ENTR_TEST01_0`) or the Lost Woods overworld flame when
`CVAR_TRIFOOL_FOUNTAIN_OVERWORLD` is on.

## Files

- `UnicornFountain.blend` — scene, room, water box, lighting, Triforce%
  custom-actor empties (stripped on export).
- `UnicornFountain_scene.out.ld` — Fast64 linker output stub.
- `conf.txt` — Fast64 scene flags (shader/save slots).
- `textures/`, `title.ia8.png` — scene textures and the title card baked into
  `TrifoolPercent.o2r`.

## Export flow

The export script lives at `scenes/unicorn_fountain/export_fountain.py` (lower
case — tooling vs. asset folders are intentionally split). It runs headless
through `bpy`, strips Triforce%'s hijacked-id custom actors, re-parents the
entrance to the room, mutes vanilla music, and writes the resources into the
Test01 slot.

Run from Isaac's machine (the Blender/Fast64 venv is only set up there — a
cloud session cannot do the export):

```
.venv-blender/bin/python scenes/unicorn_fountain/export_fountain.py \
    --blend third_party/triforce-percent/scene/UnicornFountain/UnicornFountain.blend \
    --fast64 third_party/addons \
    --soh-include third_party/Shipwright/soh \
    --out build/o2r
```

The result lands under `<out>/scenes/shared/test01_scene/`, which the top-level
`.o2r` packer picks up.

### F3D materials

Scene geometry must use F3D materials, not EEVEE/Cycles. PR #30 CI failed on
this; the fix is to open `UnicornFountain.blend` in Blender with the HM Fast64
addon enabled and run **Fast64 → Material → Convert to F3D** on every mesh
before re-exporting.

### Visible Great-Fairy placeholder (new)

The custom `Tf_Beta_Great_Fairy` actor (`soh/.../actors/z_tf_beta_great_fairy.c`)
is a port of Triforce%'s `BetaGreatFairy` state machine. The Triforce% team
never published the fairy's zobj, so right now the ported actor runs mesh-less
— a column of golden sparkle particles rises, the Overture-of-Sages text plays,
and the fade-back fires on cue, but there is no visible fairy body.

The export script now also spawns a **vanilla Great Fairy actor** at the
fountain centre `(0, 0, 0)` with params `0x0000`, and adds the fairy's object
to the room's object list, so the scene has an actual on-screen body while the
ported state machine keeps owning the dialogue and exit trigger. The two live
side by side; the vanilla NPC is scenery only. To tweak the spawn, edit the
`PLACEHOLDER_FAIRY_*` constants at the top of `export_fountain.py` and
re-export.

> **Verify the actor/object names before relying on the export.** The script
> currently uses `ACTOR_EN_GE1` and `object_ge1`, following the brief that
> framed this slice. Those identifiers may not match the Great-Fairy actor
> you actually want — grep Shipwright's `soh/include/tables/actor_table.h`
> and `object_table.h` for the right pair (e.g. the Great Fairy who gives
> Giant's Knife / spells) and swap the two constants if needed. Fast64 will
> reject an unknown identifier at export time with a clear error.

## See also

- `scenes/TestRoom/README.md` — the hex hub this fountain is reached from.
- `soh/Enhancements/TrifoolPercent/UnicornFountain.cpp` — runtime glue
  (Overture-of-Sages dialogue, entry CS trigger, fade-back on fairy done).
- `soh/Enhancements/TrifoolPercent/actors/z_tf_beta_great_fairy.c` — the
  ported state machine, including the three porting gaps still open (zobj,
  scene cutscene, song gift).
