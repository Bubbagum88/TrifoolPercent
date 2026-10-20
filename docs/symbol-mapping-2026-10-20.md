# Symbol mapping: `func_80XXXXXX` → Shipwright names (2026-10-20)

Audit of every `func_80[0-9A-Fa-f]{6}` reference in this repo against the
Shipwright commit this project pins (`SHIPWRIGHT_COMMIT=6b4edce` in
`tools/setup.sh`, 2026-10-14 on `develop`).

## TL;DR

The compiled code lives under `soh/Enhancements/TrifoolPercent/`. Across that
tree there were **only three** `func_80…` call sites, all in
`actors/z_tf_beta_kokiri.c`. Both symbols these call sites used are **absent
from Shipwright at the pinned SHA**; they are OoT NTSC 1.0 ROM addresses from
the original Triforce% ASM-patch scripts. They are replaced with the
Shipwright/decomp names in this change:

| Old name (NTSC 1.0 addr) | Shipwright name           | Signature                                                                 |
| ------------------------ | ------------------------- | ------------------------------------------------------------------------- |
| `func_80034A14`          | `Npc_TrackPoint`          | `void(Actor*, NpcInteractInfo*, s16 presetIndex, s16 trackingMode)`       |
| `func_80078884`          | `Sfx_PlaySfxCentered`     | `void(u16 sfxId)`                                                         |

Both are declared in `third_party/Shipwright/soh/include/functions.h`
(lines 537 and 984 at the pinned SHA).

## Legacy / reference code (NOT compiled)

The `actors/`, `cutscenes/`, `scenes/` directories at the repo root are the
upstream Triforce% N64-ROM sources kept for reference; they are not fed to
the SoH build. They still contain the following `func_80XXXXXX` references
(all NTSC-1.0 addresses). Each is paired below with the symbol in Shipwright
at the pinned SHA.

### Present in Shipwright with the same name (keep as-is)

Shipwright is based on OoT **MQ Debug**, so a `func_80XXXXXX` that still
appears there has by coincidence the same address in both ROMs. These do
not need renaming when we port the file into `soh/Enhancements/`:

| Symbol           | Where in Shipwright                                  |
| ---------------- | ---------------------------------------------------- |
| `func_8002829C`  | `soh/include/functions.h:234`                        |
| `func_80034BA0`  | `soh/include/functions.h:538`                        |
| `func_80034CC4`  | `soh/include/functions.h:540`                        |
| `func_8003EBF8`  | `soh/include/functions.h:653`                        |
| `func_8003EC50`  | `soh/include/functions.h:654`                        |
| `func_808519EC`  | `soh/src/overlays/actors/ovl_player_actor/z_player.c` (static csAction) |
| `func_80851A50`  | `soh/src/overlays/actors/ovl_player_actor/z_player.c` (static csAction) |

### Absent from Shipwright — resolve by semantics before porting

These are NTSC-1.0 ROM addresses whose Shipwright/MQ-Debug counterpart has
a different address (or has been decompiled under a semantic name). When
porting the referencing file into `soh/Enhancements/`, replace by the
decomp name below rather than the raw address. The resolutions marked
**verified** were cross-checked against `functions.h` at the pinned SHA;
the ones marked *(to confirm)* are the common OoT-decomp names and have
not yet been verified against the pinned Shipwright tree.

| 1.0 symbol       | Shipwright/decomp name                       | Notes |
| ---------------- | -------------------------------------------- | ----- |
| `func_80034A14`  | `Npc_TrackPoint`                             | verified |
| `func_80078884`  | `Sfx_PlaySfxCentered`                        | verified |
| `func_8002F698`  | *(to confirm — Actor_SetColorFilter?)*        | 7-arg `(play, actor, f32, s16, f32, u8, u8)` |
| `func_8002F71C`  | *(to confirm — Actor_PlaySfx?)*               | `(play, actor, f32, s16, f32)` |
| `func_80037F30`  | *(to confirm)*                               | `(Vec3s* headRot, Vec3s* torsoRot)` |
| `func_80038290`  | *(to confirm)*                               | `(play, actor, headRot, torsoRot, ...)` |
| `func_80043590`  | *(to confirm — DynaPolyActor_IsInRidingState / is-standing-on-ice?)* | returns bool from `DynaPolyActor*` |
| `func_80093D18`  | `Gfx_SetupDL_25_Opa` *(see comment in `z_tf_mimic_chest.c`)*  | gfx-ctx setup for opaque DL25 |
| `func_80093D84`  | `Gfx_SetupDL_25_Xlu` *(symmetric)*            | gfx-ctx setup for translucent DL25 |
| `func_8010B680`  | `Message_StartTextbox` *(see comment in `wsrcutscene.c`)*     | `Message_StartTextbox(play, textId, actor)` |
| `func_8010BDBC`  | *(to confirm — Message_GetState?)*            | `(msgCtx)` returning msg state |
| `func_80831758`  | `Player_ItemToActionParam` *(see `actors/z_player/actorpatch.txt:28`)* | overlay-local; may be static in `z_player.c` |
| `func_8083C678`  | `Link_PressAOnInteractedActor` *(see `actorpatch.txt:22`)*            | overlay-local |
| `func_8083E5A8`  | counterpart of `func_8083C678` in 1.0->Debug mapping *(same semantic)* | overlay-local |
| `func_808454D8`  | `Link_GetDoActionForInteractedActor` *(see `actorpatch.txt:25`)*       | overlay-local |
| `func_808473D4`  | counterpart of `func_808454D8` in 1.0->Debug mapping *(same semantic)* | overlay-local |
| `func_8084F868`  | `Link_Action_MasterSword_Init` *(see `actorpatch.txt:6`)*             | overlay-local |
| `func_8084F8CC`  | `Link_Action_MasterSword_Update` *(see `actorpatch.txt:10`)*          | overlay-local |

Note: the player-ovl symbols (`808xxxxx`) are typically **static** inside
`z_player.c`, so they cannot be called by name from a separate TU anyway.
When a feature needs to patch one, it has to use SoH's hook / detour API,
not a direct call.

## Why this happened

The upstream Triforce% project is a ROM-asm patch set for OoT NTSC 1.0.
Function references in it are literal `func_<1.0-ROM-addr>` labels. When
code was copied into `soh/Enhancements/TrifoolPercent/`, those labels came
along even though the SoH build targets Shipwright (MQ Debug + decomp
names). The labels compile only when:

  * the same `func_80XXXXXX` is still declared in Shipwright's
    `functions.h` (the seven in the first table above), **or**
  * the file is upstream-only reference (not in the build), which is why
    only three real call sites were affecting the build.

## How to redo this check

```bash
# From the repo root
grep -rnE '\bfunc_80[0-9A-Fa-f]{6}\s*\(' soh/Enhancements/
```

To look a symbol up in the pinned Shipwright headers:

```bash
SHA=$(grep '^SHIPWRIGHT_COMMIT=' tools/setup.sh | cut -d= -f2 | cut -d' ' -f1)
curl -sS "https://raw.githubusercontent.com/HarbourMasters/Shipwright/$SHA/soh/include/functions.h" \
  | grep -nE '\b<name>\b'
```
