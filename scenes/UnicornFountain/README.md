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

### Open: visible Great-Fairy body (zobj not ported)

The custom `Tf_Beta_Great_Fairy` actor (`soh/.../actors/z_tf_beta_great_fairy.c`)
is a port of Triforce%'s `BetaGreatFairy` state machine. The Triforce% team
never published the fairy's zobj, so right now the ported actor runs mesh-less
— a column of golden sparkle particles rises, the Overture-of-Sages text plays,
and the fade-back fires on cue, but there is no visible fairy body.

Follow-ups that would give the scene a body on screen:

- **Port the zobj.** The real fix. Convert the BetaGreatFairy DL from the
  Triforce% assets (if the mesh data can be recovered) into an `object_*`
  entry under `assets/objects/` and have the ported actor draw it.
- **Spawn a vanilla Great Fairy as scenery.** A placeholder NPC in the pool
  while the real zobj is pending. Earlier attempts tried `ACTOR_EN_GE1` /
  `obj_ge1`, but those are the Gerudo pair — the real Great-Fairy actors
  are `ACTOR_EN_OKARINA_EFFECT` / Great-Fairy specific objects. Needs the
  correct `ACTOR_*` / `obj_*` identifiers verified against Shipwright's
  `soh/include/tables/actor_table.h` and `object_table.h` (Fast64 rejects
  unknown keys at export time with a clear enum-not-found error) before
  landing a change to `export_fountain.py`.

## See also

- `scenes/TestRoom/README.md` — the hex hub this fountain is reached from.
- `soh/Enhancements/TrifoolPercent/UnicornFountain.cpp` — runtime glue
  (Overture-of-Sages dialogue, entry CS trigger, fade-back on fairy done).
- `soh/Enhancements/TrifoolPercent/actors/z_tf_beta_great_fairy.c` — the
  ported state machine, including the three porting gaps still open (zobj,
  scene cutscene, song gift).
