# TrifoolPercent Timeline

A phase-by-phase milestone view of the project. For dated entries on how
things actually went, read the [dev log](devlog.md); for the mechanic-level
checklist, read the [porting checklist](porting-checklist.md); for the full
plan, read the [roadmap](roadmap.md).

Legend: ✅ done · 🚧 in progress · ⏳ not started

---

## Phase 0 — Foundation ✅

**Window:** project start → first research doc merged (PR #1).

Achieved:
- Repo and folder layout (`assets/`, `scenes/`, `actors/`, `cutscenes/`,
  `scripts/`, `soh/`, `build/`, `docs/`, `tools/`).
- Triforce% source read end to end and mapped onto SoH equivalents
  ([code-breakdown.md](references/triforcepercent/code-breakdown.md), PR #1).
- Reference indexes created: assets, creators, import status.
- README written, dead reference links fixed, SoH tools section added
  (PR #2).
- Roadmap expanded into a phased plan from PoC to release (PR #3, #4, #7).
- Reference screenshots committed (PR #12).
- CREDITS.md (PR #9).

Current status: ✅ complete.

---

## Phase 1 — Proof of concept: the Triforce Room 🚧

**Goal:** prove the four pipelines every later scene needs — scene, actor,
object, text — on the smallest meaningful piece of the run.

Achieved:
- Shipwright fork with a local build and an extracted `oot.o2r`.
- `TrifoolPercent` SoH module gated behind a single CVar, one
  `RegisterShipInitFunc` entry point
  ([`soh/Enhancements/TrifoolPercent`](../soh/Enhancements/TrifoolPercent)).
- Triforce Room scene re-exported with HarbourMasters' Fast64 fork, packed
  into the unused `besitu` ("Test Room") debug scene slot in
  `TrifoolPercent.o2r` (PR #8, moved off Goron Shop in commit `4f0081f`).
- Three actors ported to C and registered with `ActorDB` at runtime:
  `TriforcePiece`, `TriforceChest`, `TriforceLight`.
- Objects for all three built with Torch from Triforce%'s compiled
  `zobj.zobj` files (not re-exported from the `.blend`), so they match the
  run byte for byte.
- "You got the Triforce!!!" (`0x0ACE`) and the goddesses' wish
  (`0x0E00`–`0x0E07`) wired through `CustomMessageManager` / `OnOpenText`.
- Loading zone: blue flame by Link's house in Kokiri Forest sends Link into
  the room. Debug map select "116:Test Room" also goes there.
- Real collision rebuilt from room geometry (walls, not just floors), water
  box in the pool, indoor lighting, title card, exit back to Kokiri Forest
  (commit `c6b0212`).
- Windows CI build publishing the `.o2r` as an artifact; Shipwright
  submodules fetched by SHA for reproducibility (commit `57b6819`).
- Porting checklist created so every remaining mechanic is tracked (PR #11).

In-game verification (2026-10-15, SoH 9.2.3 Windows, `.o2r` only, no module):
- ✅ Scene loads in the `besitu` slot.
- ✅ Walls, water, exit and title card all work.
- ⏳ Actors, cutscene, text — not yet checked. Needs a Windows SoH build
  with the module linked in.

Open items:
- ⏳ `docs/build.md` with reproducible local build steps.
- ⏳ Resolve Link's cutscene actions `0x4B`, `0x4E`, `0x4F`, `0x50`
  (Triforce% used custom animations here; stock SoH will fall back to
  vanilla).
- ⏳ Verify TriforceLight's flex skeleton and the room's materials through
  a current Fast64/Torch pass in a running game.

Current status: 🚧 in progress. Exit criterion: walk through the blue flame,
watch the three pieces rise, combine and sink into Link, and get the Triforce
text box in a running game.

---

## Phase 2 — Toolchain and workflow ⏳

**Goal:** make Phase 1 reproducible before scaling it up.

Planned:
- One script that builds the `.o2r` from this repo's sources end to end.
- Decide how this repo and the Shipwright fork stay in sync (submodule, or
  code kept only in the fork).
- Debug warps mirroring `Statics_TestShortcuts` for every story beat.
- Build the Triforce% romhack ROM as a side-by-side reference.
- CI that at least builds the fork.
- **Check whether SoH can load compiled code from a mod package.** This
  decides whether the Shipwright fork can go away eventually, so it shapes
  all later organisation.
- Decide the HD texture mechanism (separate `.o2r` vs. SoH alternate-assets
  toggle) and add the HD export to the build script.
- Convert `run/triforcepercent_data.run` into a tracked porting status doc.

Exit criterion: a fresh clone can produce a playable Triforce Room build by
following `docs/build.md`.

Current status: ⏳ not started.

---

## Phase 3 — The Sacred Realm ⏳

**Goal:** prove custom songs, warps and in-engine cutscenes.

Planned: Overture of Sages (new ocarina song), WSR warp from the Temple of
Time, Chamber of Sages scene, `CoSStaircases` and `CoSSages` actors with the
sage dialogue, first custom music (`ChamberOfSages.aseq`, `triforce.aseq`,
`PowerOfTheGoddesses.aseq`), the goddesses' wish choices.

Exit criterion: play the Overture in the Temple of Time and reach the
Triforce Room from it with no debug help.

Current status: ⏳ not started.

---

## Phase 4 — Key of Wisdom ⏳

**Goal:** Zora's Domain → Unicorn Fountain → Beta Great Fairy teaches the
Overture.

Planned: Zora's Domain ice-melt, Domain → Lake Hylia exit rerouted to the
Unicorn Fountain (Granny's Potion Shop slot), Unicorn Fountain scene,
Pedestal of the Ocarina, Beta Great Fairy actor and cutscene.

Exit criterion: from a fresh file, learn the Overture of Sages from the Beta
Great Fairy.

Current status: ⏳ not started.

---

## Phase 5 — Keys of Courage and Power ⏳

**Goal:** the two side-quest chains.

Planned (Courage): new-file game state, Navi silence, Lost Woods secret path,
Beta Kokiri, Butterfly in the Bazaar, rewritten Skull Kid (Bunny Hood +
Gerudo Mask upgrades), Gerudo guards and Thieves' Hideout, Nabooru with the
shehnai and the "full Song of Time", time travel anywhere.

Planned (Power): Running Man race rewrite, Running Man boss (needs new
model), Sages' Charm (Medallions on C, Fire Medallion as Din's Fire, Zora's
River warp).

Exit criterion: all three keys earnable in order on one save file.

Current status: ⏳ not started.

---

## Phase 6 — Ending, staff roll and polish ⏳

Planned: Ending scene in Dampé's Hut slot, `FinaleCS`, Silent Princess
flowers, Holy Light, BotW Link and Zelda (new models), hair and tunic
physics, cel shading (stretch), remaining voice clips and music, staff-roll
flyovers with the Triforce% team credited, optional real scene-table entries,
optional `TwitchMessages`.

Exit criterion: a full playthrough from new file to the end of the staff
roll.

Current status: ⏳ not started.

---

## Phase 7 — Release ⏳

Planned: full-quest playtest, soft-lock and flag fixes, confirm vanilla
saves and SoH features still work with the CVar off, credits and licensing
pass, drag-and-drop release package (see roadmap for the three options),
GameBanana listing, community testing.

Exit criterion: a release a player can install into stock SoH's `mods` folder
and play from new file to credits.

Current status: ⏳ not started.

---

## Capability milestones

Smaller, cross-phase moments worth marking as they land:

| Milestone | Status | Notes |
|---|---|---|
| Custom scene loads in SoH from an `.o2r` | ✅ | Triforce Room in `besitu` slot. |
| Custom collision + water + indoor lighting in a custom scene | ✅ | Commit `c6b0212`. |
| Custom title card in a scene without one | ✅ | `TitleCard_InitPlaceName` on a built texture. |
| Custom actors registered with `ActorDB` | ✅ | Code compiles; in-game run pending. |
| Custom actors spawning and animating in-game | ⏳ | Needs module linked into a Windows SoH build. |
| `CustomMessageManager` text in a custom cutscene | ⏳ | Code wired; needs in-game run. |
| Entrance rerouting via `Statics_SetUpRouting` equivalent | ⏳ | Blue-flame loading zone is a stand-in. |
| Custom ocarina song and warp hijack | ⏳ | Phase 3. |
| Custom music sequence loaded in SoH | ⏳ | Phase 3. |
| Custom item (icon + name + behaviour) | ⏳ | Phase 5 (Butterfly, Sages' Charm). |
| HD-texture variant builds alongside the base `.o2r` | ⏳ | Phase 2 decision. |
| SoH build with module ships from CI | 🚧 | `.o2r`-only CI exists; module build pending. |
| Drag-and-drop install (no custom SoH build) | ⏳ | Phase 2 decides which of the three paths. |
