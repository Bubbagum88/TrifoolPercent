# TrifoolPercent Dev Log

A running log of what has been tried, what has worked, what has broken, and
why decisions were made the way they were. Written in reverse chronological
order (newest entry on top). The goal is simple: if someone picks this project
up tomorrow, they should be able to read down this file and avoid repeating
the mistakes already paid for.

For the big picture, read the [roadmap](roadmap.md); for the current
mechanic-by-mechanic status, read the [porting checklist](porting-checklist.md);
for phase-level milestones, read the [timeline](timeline.md).

A notebook companion lives at [`devlog.ipynb`](devlog.ipynb). GitHub renders
it natively, and it has small runnable cells (list the current `.o2r`'s
resources, show reference screenshots). This Markdown file is still the
canonical narrative; the notebook mirrors it and adds media. See the
notebook's top cell for the tradeoffs.

---

## 2026-10-15 — Catching up: where the project stands today

Snapshot of the state the project is in as this log begins.

**What runs end-to-end.** The Triforce Room proof of concept, in the unused
`besitu` ("Test Room") debug scene slot, loaded by walking through a blue
flame added by Link's house in Kokiri Forest. The scene exports cleanly, the
room has real collision (walls, not just floors), a water box in the pool,
indoor lighting, a title card, and an exit back to Kokiri Forest. Tested on a
stock Ship of Harkinian 9.2.3 Windows build with the `.o2r` only (no module),
on 2026-10-15.

**What is written but not yet run in-game.** The SoH module itself: the three
ported actors (`TriforcePiece`, `TriforceChest`, `TriforceLight`) registered
with `ActorDB`, the cutscene, and the `0x0ACE` / `0x0E00`–`0x0E07` text pushed
through `CustomMessageManager`. The module compiles clean against Shipwright
`6b4edce` (2026-10-14) but has not been exercised in a running game yet because
no Windows build of SoH with the module linked in has been produced. The two
most likely rough spots when that finally runs:

- Link's cutscene actions `0x4B`, `0x4E`, `0x4F`, `0x50`. Triforce% patched
  these with custom animations (Meditate, ReachUp). On stock SoH, Link will do
  whatever vanilla assigns to those numbers, which is probably wrong.
- The TriforceLight's flex skeleton and the room's materials going through a
  current Fast64 and Torch for the first time. The source `.blend` is from
  2021 and the export script already does a round of fixups to make it load.

**What has been researched but not touched.** Everything beyond Phase 1:
Chamber of Sages, Overture of Sages, Unicorn Fountain, the three Keys quest
chains, the ending, the staff roll. The porting checklist enumerates every
item.

### Decisions that already shape the project

- **Code module + `.o2r`, not a single package.** New actor behaviour has to
  be compiled into SoH, so a Shipwright fork is unavoidable today. Assets live
  in `TrifoolPercent.o2r` and go in SoH's `mods/` folder. The roadmap keeps
  three options open for closing the "drag-and-drop install" gap later: SoH's
  C mod system (libultraship's `manifest.json` + events), upstreaming the
  module, or shipping a bundled build. Phase 2 is where we find out which one
  is realistic — do not design around any assumption here yet.
- **Scene slots over new scene-table entries.** The Triforce Room lives in
  `besitu` (map select "116:Test Room"); planned later: Granny's Potion Shop
  for the Unicorn Fountain, Dampé's Hut for the Ending. Real scene-table
  entries are a Phase 6 cleanup, not a Phase 1 problem. Rewiring entrances
  with `Statics_SetUpRouting` is the pattern for sending Link into them.
- **Reuse before porting.** Bunny Hood speed/jump, time travel without the
  Master Sword, custom music loading, and the save editor are already in SoH;
  the Arwing is vanilla `En_Clear_Tag` with a spawn trigger. The porting
  checklist marks each of these **(already in SoH)** so nobody re-implements
  them.
- **One source, two exports for textures.** Each texture's master lives at HD
  resolution in `assets/textures/`; the N64 version is generated from it. The
  HD pack ships as a separate optional `.o2r` next to the base one. Final
  mechanism (separate archive vs. SoH alternate-assets toggle) is a Phase 2
  question.
- **Custom save flags belong in SoH's `SaveManager` custom section**, not in
  borrowed `infTable` / `itemGetInf` bits. The randomizer uses those bits.

### Mistakes already paid for, so you don't have to

- **Do not hijack the Goron Shop slot.** The first PoC did and the Goron Shop
  stopped being vanilla. Moved to `besitu` (Test Room) in commit `4f0081f`.
  Any debug-only slot that is not reached in normal play is a safer home for a
  prototype scene.
- **Collision from the original Triforce% `.blend` is floors and stairs
  only.** Walking around in it is painful: Link falls off the walkway, the
  pool has no water box, there is no exit. The export script now rebuilds
  collision from the visible room geometry (walls included), adds a water
  box, drops a loading zone at the end of the entry walkway, and marks the
  scene indoor (no sky/sun/moon, clock stops). This was commit `c6b0212`,
  "real collision, water, exit, indoor lighting and title card".
- **The Triforce% `.blend` references textures that were never published.**
  Five textures (sandstone, floor tiles, water layers) are not in the repo —
  they came from the ROM or beta material. The export falls back to
  `make_placeholder_textures.py`; dropping the real PNGs into
  `third_party/triforce-percent/scene/TriforceRoom/textures/` under the
  expected filenames and rebuilding replaces them. Shipping with placeholders
  is fine for the PoC but needs cleanup before release.
- **Animation ticks that live in `Draw` double up with SoH's frame
  interpolation.** TriforceLight's animation originally advanced in `Draw`;
  with SoH's interpolation on, it ran at the wrong speed. The port moves the
  advance into `Update`. Rule of thumb for every ported actor: anything that
  mutates state belongs in `Update`, not `Draw`.
- **Actor slot overwrites don't translate.** Triforce% overwrote slots 1, 3
  and 5. In SoH that collides with other actors and offers nothing in return,
  because `ActorDB` lets you register new actors at runtime with their own
  ids. Spawn them from `OnSceneSpawnActors` rather than from the room's
  compiled actor list, because their ids aren't known until runtime.
- **Objects taken from Fast64 re-export don't always match byte-for-byte.**
  For the three Triforce actors we use Triforce%'s already-compiled
  `zobj.zobj` files directly, fed through a small `make_torch_input.py` that
  writes the YAML Torch needs. The actors then load their resources by path
  and need no object slot. Re-exporting them from the `.blend` only invites
  drift.
- **Fast64's HarbourMasters fork is the one to use.** Not stock Fast64. The
  export script runs it headless by importing Blender as the `bpy` Python
  module.
- **Shipwright submodules must be fetched by SHA in CI.** The initial CI run
  tried to rely on `actions/checkout` for submodules and the pins drifted.
  Commit `57b6819` moved to explicit SHA fetches so builds are reproducible.
- **Scene-slot behaviour follows the slot, not the mod.** The Triforce Room
  was first in the Goron Shop slot and inherited the shop's "no items in
  here" behaviour, which was convenient by accident. In `besitu`, whether
  items are blocked is an unanswered question — the slot's own scene flags
  decide, not the room resources.

### Toolchain decisions worth keeping

- **`.o2r` build runs from this repo without touching Shipwright.** The
  `tools/build_o2r.sh` script fetches pinned Triforce%, Shipwright and HM
  Fast64 sources plus a Python venv with Blender's `bpy`, then produces
  `build/TrifoolPercent.o2r`. CI publishes it as an artifact. Normal
  iteration does **not** rebuild SoH itself — only when the module in
  `soh/Enhancements/TrifoolPercent/` changes does `tools/build_soh.sh` need
  to run, and that is the one slow step.
- **One CVar, one entry point.** The module registers through a single
  `RegisterShipInitFunc` and gates everything behind an Enhancements >
  TrifoolPercent toggle. Vanilla SoH saves must keep working with it off.
- **Documentation lives in the repo.** The code breakdown
  ([docs/references/triforcepercent/code-breakdown.md](references/triforcepercent/code-breakdown.md)),
  the roadmap, the porting checklist, the PoC design doc and this log are all
  the handoff material a new contributor needs.

### Open questions carried forward

- Does SoH's current C mod system load compiled code from a mod package? If
  yes, the Shipwright fork can eventually go away. **Check in Phase 2.**
- Cel shading for BotW Link/Zelda: no RSP microcode in SoH, so this is
  shader-side in libultraship or a cheaper lookalike. Stretch goal.
- Running Man boss skeleton and BotW Link/Zelda meshes are **asset missing**
  in the upstream repo and will need original replacements or stand-ins. Not
  blocking until Phase 5–6.
- HD texture mechanism: separate `.o2r` vs. SoH's alternate-assets toggle. To
  decide in Phase 2.

---

## How to add an entry

When you finish a chunk of work, add a new section at the top of this file:

```
## YYYY-MM-DD — Short title

What was done (one or two sentences).

### What worked
- …

### What broke, and the fix
- …

### Decisions made
- …

### Open questions
- …
```

Favour honesty over polish. If something took three tries, say so and say why
the first two didn't work — that is the whole point of this log.
