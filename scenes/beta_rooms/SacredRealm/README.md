# Sacred Realm (Modified) — beta/debug room

Starting point for the TrifoolPercent "Modified Sacred Realm": a copy of
vanilla **Chamber of the Sages** (`kenjyanoma_scene`, scene slot `0x44`
= decimal 68, SoH `SCENE_CHAMBER_OF_THE_SAGES`) with all of its stairs,
platforms, warp pads and lighting kept intact, then populated with
beta/unused actors from the TCRF dump so the room doubles as a static
asset gallery. Animation + Overture-of-Sages wiring come in a later
pass — this scaffold only covers geometry reuse and static actor
placement.

The sibling `scenes/ChamberOfSages/` directory holds the Fast64
`.blend` + exported textures for the *finished* Sacred Realm scene we
want to ship with the mod proper. Keeping the beta-room build in a
separate directory means edits here never risk clobbering that
"production" source tree.

## Build approach

Two production paths are viable and both are preserved by this
scaffold; the one we actually use is decided when Isaac next runs the
local Blender/toolchain pass:

1. **Vanilla-patch path (preferred for a first pass)** — mirror the
   `scenes/tokinoma/` pattern and apply a binary patch to the vanilla
   `scene/68/scene.zscene` + `scene/68/room_0.zmap` in-place so the
   room header gains the beta object slots and the actor list gains the
   new spawns. Nothing else about the vanilla scene changes. See
   `scene_patch.txt` + `room0_patch.txt` for the hex-offset layout and
   the per-field notes. The patches here are **placeholders**:
   offsets, counts and the pre-existing byte sequences need to be read
   off the vanilla `.zscene`/`.zmap` on Isaac's build machine before
   `make` will produce a correct output.
2. **Fast64 re-export path** — bring `scenes/ChamberOfSages/
   ChamberOfSages.blend` into Blender, add the actor/object entries
   listed below to the scene header, and export over scene slot 68.
   This is the long-term plan once the room gets custom geometry; it
   is overkill for a pure "add some static actors" pass.

The `Makefile` here currently builds nothing (`default:` is a no-op)
so this directory can land safely while the patch bytes are still
TODO. Un-comment the `MAKESUBDIRS` entry in `scenes/Makefile` and
fill in the real offsets to turn it on.

## Loading-zone wiring

The test-room doorway labelled "Modified Sacred Realm" (doorway 3 in
`scenes/TestRoom/export_test_room.py`) already warps to
`ENTR_CHAMBER_OF_THE_SAGES_0`. Because this scene *patches the vanilla
scene slot in-place*, the existing exit lands in our modified room
with no further wiring — Isaac doesn't need to edit the test-room
`.o2r`. `soh/Enhancements/TrifoolPercent/TestRoom.cpp` just gets its
sign text updated to describe the new content.

The vanilla gameplay path that reaches Chamber of the Sages after
pulling the Master Sword also lands in the modified scene. Any future
`CVAR_TRIFOOL_` toggle that wants to gate that is out of scope for
this pass; the plan is to live with the shared slot during
development and move to a dedicated non-vanilla slot (e.g. one of the
remaining SoH test slots) once the beta content stops being purely
decorative.

## Object slots to add to the room header

The vanilla Chamber of Sages room header carries only the small set
of objects its native actors need. The beta/unused actors below drag
in their own objects; each must appear in the room's object list
before the actor's `Init` will succeed. Object IDs taken from
`z_actor_table.h` / TCRF.

| Object ID | Symbol               | Needed by                                                |
|-----------|----------------------|----------------------------------------------------------|
| `0x00F9`  | `OBJECT_BIRD`        | `ACTOR_EN_BIRD` (0x0072) — unused blue swallow           |
| `0x009A`  | `OBJECT_VASE`        | `ACTOR_EN_VASE` (0x0082) — beta Hyrule Castle vase       |
| `0x009B`  | `OBJECT_NWC`         | `ACTOR_EN_NWC` (0x0135) — unused chick                   |
| `0x000F`  | `OBJECT_FIRE`        | `TfUnusedFlame` billboard (vanilla "Dancing Flame")      |

Note: the catalogue of beta actors advertised in `TestRoom.cpp`
(`kTestRoomExhibits`) is wider than this list. The others (`En_Part`,
`En_Scene_Change`, `En_OE2`, `En_Horse_Zelda`, `En_Lightbox`,
`En_Pu_box`, `En_Clear_Tag`, `Item_Inbox`, `Obj_Elevator`) are
plaque-only exhibits for now — this room places the four with easy
drop-in geometry; the rest come in a second pass once Isaac confirms
spawn parameters that don't instantly crash. `En_Nwc` is listed as
object `0x009B` on the OoT side; TrifoolPercent's existing
`z_tf_unused_flame.c` already proves out the "ship unused object +
spawn invisible actor" pattern.

## Actor spawn table

Positions are in OoT world units, relative to the vanilla Chamber of
Sages room origin (centre of the warp pad). Y matches the floor
height for the stair tier named in the comment. Rotations are degrees
*16.
Birds get an idle `params=0x0001` so they ride their default flight
AI around spawn; the chick, vase and dancing flame all accept
`params=0x0000`.

| #  | Actor ID       | Object   | Pos (X, Y, Z)       | Rot Y | Params   | Where on the Sacred Realm floor                  |
|----|----------------|----------|---------------------|-------|----------|--------------------------------------------------|
| 1  | `0x0072` Bird  | `0x00F9` | ( +400, +600, -600) | 0x4000 | `0x0001` | Above the Sage platform ring, SE quadrant        |
| 2  | `0x0072` Bird  | `0x00F9` | ( -400, +600, -600) | 0xC000 | `0x0001` | Above the Sage platform ring, SW quadrant        |
| 3  | `0x0072` Bird  | `0x00F9` | (    0, +800, +600) | 0x8000 | `0x0001` | Above the Sage platform ring, N  quadrant        |
| 4  | `0x0082` Vase  | `0x009A` | ( +260,    0, -40)  | 0x0000 | `0x0000` | Bottom of the pedestal steps, east railing       |
| 5  | `0x0082` Vase  | `0x009A` | ( -260,    0, -40)  | 0x0000 | `0x0000` | Bottom of the pedestal steps, west railing       |
| 6  | `0x0135` Nwc   | `0x009B` | (  +80,    0, +80)  | 0x4000 | `0x0000` | Beside Rauru's spot, south edge of warp pad      |
| 7  | `0x0135` Nwc   | `0x009B` | (  -80,    0, +80)  | 0xC000 | `0x0000` | Beside the Light Sage spot, south edge           |
| 8  | TfUnusedFlame  | `0x000F` | ( +260, +40, +260)  | 0x0000 | `0x0000` | On top of the east pedestal post                 |
| 9  | TfUnusedFlame  | `0x000F` | ( -260, +40, +260)  | 0x0000 | `0x0000` | On top of the west pedestal post                 |
| 10 | TfUnusedFlame  | `0x000F` | (    0, +40, -260)  | 0x0000 | `0x0000` | On top of the north pedestal post                |

`TfUnusedFlame`'s own actor id is runtime-assigned (see
`TrifoolPercent.cpp`); for the room-header path it is spawned by
`ActorDB::AddEntry` in the SoH process, so the room file lists only
`OBJECT_FIRE` (0x000F) and TrifoolPercent's init hook injects the
flames once the scene reports as loaded. The vase/chick/bird spawns
go straight into the vanilla actor list via the patch below.

Numbers are intentionally conservative — the Chamber of Sages playfield
has a usable radius of about 400 OoT units at floor height; the Sage
platform ring sits roughly 600 units above the floor. Fine-tuning
belongs to the Blender re-export pass.

## Follow-ups (deliberately out of scope for this pass)

- Animations (bird flight loops, chick wander, flame flicker is
  already provided by the TfUnusedFlame draw path).
- Overture-of-Sages cutscene wiring (`docs/cutscene-plan.md` §3.4).
- Dedicated non-vanilla scene slot so the "modified" and "vanilla"
  Chambers of Sages can coexist.
- Plaques for the remaining beta actors (`En_Part`, `En_Horse_Zelda`,
  …) once we pick parameters that behave in-scene.
