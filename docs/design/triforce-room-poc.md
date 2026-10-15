# Proof of concept: the Triforce Room

The first TrifoolPercent milestone rebuilds Triforce%'s Triforce Room inside Ship of Harkinian. Walk through the blue flame by Link's house in Kokiri Forest and you get the room, the chest, the three Triforce pieces rising and joining, "You got the
Triforce!!!" and the goddesses' wish. It proves the four pipelines every later scene needs: scene, actor,
object (model) and text.

## How it is built

TrifoolPercent is two pieces, as the [code breakdown](../references/triforcepercent/code-breakdown.md#6-recommended-architecture-for-trifoolpercent) planned:

| Piece | What it holds | Where it lives |
|---|---|---|
| SoH module | The three actors ported to C, the cutscene, the text, and the hooks that tie them together | [`soh/Enhancements/TrifoolPercent/`](../../soh/Enhancements/TrifoolPercent), copied into a Shipwright checkout |
| `TrifoolPercent.o2r` | The room scene and the three actor models | Built by [`tools/build_o2r.sh`](../../tools/build_o2r.sh) from the Triforce% repo; goes in SoH's `mods` folder |

```
triforce-percent/scene/TriforceRoom/TriforceRoom.blend --(headless Blender + HM Fast64)--> scenes/shared/besitu_scene/*  --+
triforce-percent/actor/Triforce{Piece,Chest,Light}/zobj.zobj --(Torch)--> objects/object_trifool_triforce_*/*           --+--> TrifoolPercent.o2r
triforce-percent TriforceRoom_scene.c cutscene + text/messages.c --> soh/Enhancements/TrifoolPercent (compiled into SoH)
```

### Scene: an unused debug scene slot

Triforce% put its Triforce Room in the Goron Shop's scene slot. TrifoolPercent uses one of the game's unused
debug scenes instead, `besitu` (map select "116:Test Room"), so the Goron Shop stays vanilla. SoH loads it from
`scenes/shared/besitu_scene/` in its archives, and a mod archive with those files wins. The module adds a loading
zone into the room in front of Link's house in Kokiri Forest, marked by a blue flame; the Test Room entry in the
debug map select also goes there.

[`scenes/triforce_chamber/export_room.py`](../../scenes/triforce_chamber/export_room.py) runs HarbourMasters'
Fast64 fork headless (Blender as the `bpy` Python module), which writes SoH's binary scene, room, collision,
display list, vertex and texture resources directly. The Triforce% `.blend` dates from 2021, so the script fixes
it up first (see its docstring).

Two more things are added for playing in the room rather than only watching the cutscene. Triforce%'s collision
was just the floors and stairs, so the script rebuilds it from the visible room (walls included), fills the pool
with a water box, and puts a loading zone out to Kokiri Forest at the end of the entry walkway, matching the test
entrance. It also makes the room indoor: no sky, sun or moon, and the clock stops. The debug scene has no title
card, so the module calls `TitleCard_InitPlaceName` on a "Triforce Room" texture built by
[`make_title_card.py`](../../scenes/triforce_chamber/make_title_card.py).

**The room's textures are placeholders.** The `.blend` points at five images Triforce% never published (they
appear to come from the ROM or from beta material). [`make_placeholder_textures.py`](../../scenes/triforce_chamber/make_placeholder_textures.py)
writes simple stand-ins (sandstone bricks, floor tiles, two water layers) for any that are missing. Drop the real
PNGs into `third_party/triforce-percent/scene/TriforceRoom/textures/` with the same names and rebuild to replace
them.

### Actors: ActorDB instead of hijacked slots

Triforce% overwrote actor slots 1, 3 and 5 with its TriforcePiece, TriforceChest and TriforceLight. In SoH they
are new actors registered with `ActorDB` at runtime (`Tf_Triforce_Piece`, `Tf_Triforce_Chest`,
`Tf_Triforce_Light`). Their ids are only known at runtime, so they are spawned from code when the room loads
(`OnSceneSpawnActors`) rather than from the room's actor list. The C is a near line-for-line port; the light's
animation now advances in `Update` instead of `Draw` so it does not speed up with SoH's frame interpolation.

### Objects: Torch on Triforce%'s compiled models

The actors' models are taken from the compiled `zobj.zobj` files in the Triforce% repo, not re-exported from
Blender, so they match the run byte for byte. [`make_torch_input.py`](../../actors/triforce_actor/make_torch_input.py)
walks each object's display lists and skeletons, writes the YAML that Torch (HarbourMasters' asset converter,
the same one SoH uses on your ROM) needs, and Torch writes SoH resources. The actors load them by path, so they
need no object slot.

### Cutscene and text

The room's cutscene is Triforce%'s own (exported by Fast64 into `TriforceRoom_scene.c`), converted to SoH's
cutscene macros in [`TriforceRoomCs.c`](../../soh/Enhancements/TrifoolPercent/TriforceRoomCs.c) and started
whenever the room loads. Its text (`0x0ACE` and the goddesses' `0x0E00`–`0x0E07`) is supplied through the
`OnOpenText` hook as raw OoT message bytes, copied from Triforce%'s `text/messages.c`.

## What was checked, and what was not

Checked in a Linux container on 2026-10-15:
- The room exports cleanly; the resources sit at the Besitu scene's path and the time warp actor lands at the
  right spot (0, -54, 248).
- Torch converts all three objects with every texture, palette, vertex array, limb, skeleton and animation.
- `build/TrifoolPercent.o2r` packs to 53 resources.
- The module compiles with no warnings against Shipwright `6b4edce` (2026-10-14).

Checked in the game on 2026-10-15, on a stock Ship of Harkinian 9.2.3 for Windows (the `.o2r` alone, without the
module). The room loaded in the Goron Shop slot (that was its first slot); walls, water, the exit and the title
card all work. Items in the room follow whatever rules the room's scene has: the old Goron Shop slot blocked them;
whether the Besitu slot does needs an in-game check.

**Not checked: the module in the game** (actors, cutscene, text). That needs a Windows build of SoH with the
module, which has not been made yet. The likeliest problems:
- Link's cutscene actions. The cutscene drives Link with actions `0x4B`, `0x4E`, `0x4F` and `0x50`; Triforce%
  added custom Link animations (Meditate, ReachUp) and may have patched what those actions do. On stock SoH Link
  will do whatever vanilla does for those numbers.
- The light's flex skeleton and the room's materials going through a 2026 Fast64 and Torch for the first time.

## Changes from Triforce%

| Triforce% | Proof of concept | Why |
|---|---|---|
| Room entered from the Chamber of Sages via a rerouted exit, with an entrance cutscene | Entered through a blue flame by Link's house in Kokiri Forest; the cutscene plays every visit | No Chamber of Sages yet; keeps testing to one keypress |
| Cutscene terminator `81` warps to the Ending | Fades back to Kokiri Forest at the same frame (1885) | No Ending yet; in vanilla, terminator 81 goes to Lon Lon Ranch |
| Custom music (`triforce.aseq`, `PowerOfTheGoddesses.aseq`) | Room is silent; the goddesses get the Ocarina of Time theme | Custom sequences are not ported yet |
| Beta Triforce icon in "You got the Triforce!!!" | No icon | The icon was a patched heart piece texture; needs a custom icon hook |

## Next steps

1. Build it and walk through the blue flame by Link's house in Kokiri Forest: `tools/setup.sh`, `tools/build_o2r.sh`, `tools/build_soh.sh`. On Windows, the `build-windows` CI workflow builds the whole package.
2. Replace the placeholder room textures if the originals can be found.
3. Port the two music sequences and the beta Triforce icon.
4. Chamber of Sages and the Overture of Sages, which lead into this room (step 2 of the breakdown's order of work).

## Credits

The room, actors, cutscene and text are the Triforce% team's work (Sauraen and contributors). Their license
asks that projects using their content directly credit the contributors in the in-game staff roll; a release
of TrifoolPercent has to carry that.
