# Cutscene porting plan

How we author, place and trigger every custom cutscene in TrifoolPercent. Written 2026-10-16 against this repo's state and cross-referenced with [`docs/references/soh-engine-capabilities.md`](references/soh-engine-capabilities.md) §3 (cutscene playback), [`docs/references/triforcepercent/code-breakdown.md`](references/triforcepercent/code-breakdown.md) and the Triforce% cutscene sources already vendored under `scenes/` and `actors/`.

**TL;DR.** SoH plays vanilla OoT cutscenes byte-for-byte. We write them as plain C arrays of `CS_*` macros (`soh/include/z64cutscene_commands.h`) and either (a) attach them to a scene header via `SCENE_CMD_CUTSCENE_DATA`, or (b) trigger them from a C hook with `csCtx.segment = ptr; cutsceneTrigger = 1`. Nothing in the Triforce% cutscene set needs a new command. Fast64 is the authoring tool for anything with camera motion or player/NPC action tracks; the raw macro files already in the repo are the ground truth and compile directly against SoH. **Order of work per room: scene geometry → objects/textures → actors → cutscene → entrance/trigger wiring.** A cutscene that references an actor that is not yet in `ActorDB` or an object that is not yet in the `.o2r` will crash on `Actor_Spawn` or render as a missing-model.

---

## 1. The toolchain

### What SoH consumes

A cutscene is a flat `s32[]` (aka `CutsceneData[]`) built from the macros in [`soh/include/z64cutscene_commands.h`](https://github.com/HarbourMasters/Shipwright/blob/develop/include/z64cutscene_commands.h). SoH's dispatcher is `Cutscene_ProcessCommands` in `soh/src/code/z_demo.c`. Playback respects every classic command — `CS_BEGIN_CUTSCENE`, `CS_CAM_POS_LIST` / `CS_CAM_FOCUS_POINT_LIST` (new names) or `CS_CAM_EYE_LIST` / `CS_CAM_AT_LIST` (old names), `CS_NPC_ACTION`, `CS_PLAYER_ACTION`, `CS_LIGHTING`, `CS_TEXT_DISPLAY_TEXTBOX`, `CS_TEXT_LEARN_SONG`, `CS_PLAY_BGM` / `CS_STOP_BGM` / `CS_FADE_BGM`, `CS_TIME`, `CS_SCENE_TRANS_FX`, `CS_MISC`, `CS_TERMINATOR`, `CS_END`. The three non-vanilla branches (`GameInteractor_Should` on skippable-text cases, a `BetterFarore` CVar, a `CreditsFix` on terminator cases 55–78) do not add new commands. See §3 of the engine-capabilities doc for the full list and caveats.

### Tools to author with, in order of preference

| Tool | Use for | Why |
|---|---|---|
| **Fast64** (`HarbourMasters/fast64` fork) | Everything with camera motion, NPC-action tracks or player-action tracks | Blender UI for cutscene actors, action lists and camera splines; exports the exact macros SoH consumes. All of Triforce%'s cutscenes were authored here. The repo's `.blend` files (`scenes/TriforceRoom.blend`, `scenes/Ending.blend`, `scenes/UnicornFountain.blend`, `scenes/ChamberOfSages.blend`) carry the cutscene scenes. Checkout the HarbourMasters fork, not upstream Fast64 — it tracks SoH's expectations for OoT export |
| **Hand-written C** against `z64cutscene_commands.h` | Text/BGM/light/transition-only cutscenes (staff-roll flyovers, dialogue-only intros, scene-trans fades) | A flyover with 2–3 textboxes, a BGM and a terminator is ~30 lines; the file is readable, diffable and reviewable. The twelve flyover files already in `scenes/spot*/` are this pattern |
| **SharpOcarina** (Windows) | Verifying actor placement in a scene, authoring an entrance list that references a cutscene index | A scene can be exported from Fast64 but SharpOcarina is faster for re-numbering an entrance/exit list and inspecting what cutscene index the vanilla engine will reach for |
| **z64scene** / **z64convert** / **ZAPDTR** (reference) | Reading what the vanilla engine stored, for comparison | We rarely emit through these; they are mainly for inspecting the romhack-reference ROM when a diff is ambiguous |
| **z64ovl** | *Not used.* | It targets overlay-style ACE builds. SoH loads actors through `ActorDB`, so none of this applies |
| **Triforce% `rom-setup/romhack` ROM** | Side-by-side visual reference while porting any one cutscene | Running the romhack in an emulator shows the cutscene's intended timing, music, framing and dialogue. Build it per [`docs/porting-checklist.md`](porting-checklist.md) §0 — never ship or redistribute |

### Two playback paths — pick per cutscene

1. **Scene-header cutscene (`SCENE_CMD_CUTSCENE_DATA`).** The engine runs it on scene entry when the entrance's `sceneLoadFlag`/`cutsceneIndex` matches. Used by `TriforceRoom_scene_header00` and `Ending_scene_header00` already in the repo. Also what `gEntranceCutsceneTable` reaches for.
2. **Manual trigger from C.** The pattern already in use at [`soh/Enhancements/TrifoolPercent/TrifoolPercent.cpp:131`](../soh/Enhancements/TrifoolPercent/TrifoolPercent.cpp), `actors/WSRCutscene/wsrcutscene.c:23`, and `actors/Nabooru/nabooru.c:374`:
   ```c
   gPlayState->csCtx.segment = &MyCS;
   gSaveContext.cutsceneTrigger = 1;
   ```
   Use this when the cutscene has to fire mid-scene, after a flag is set, or on an actor's state change — anything the scene header can't express. `Cutscene_SetSegment` accepts a resource-loaded cutscene too (SoH reads the `OTR` signature and calls `ResourceMgr_LoadCSByName`).

**`gEntranceCutsceneTable` is static** — new entries can't be added at runtime. For every cutscene we attach to a *custom* entrance, we go through path 2 (trigger from `OnSceneSpawnActors` or `OnTransitionEnd`) rather than through the entrance-cutscene table.

### `CS_TERMINATOR` destinations

The second arg of `CS_TERMINATOR(destination, startFrame, endFrame)` is a vanilla OoT terminator case id that picks a next entrance + fade style. SoH's `Cutscene_HandleCommand_Terminator` honours every vanilla case except `113` (hard-coded to Ganon's Tower). Triforce% chained its staff-roll flyovers by patching the terminator switch in `statics/scene.c`. We don't patch the switch — we read `csCtx.frames` and the chosen destination in an `OnPlayerUpdate` / after-cutscene handler, and set `nextEntranceIndex` + `transitionTrigger` ourselves. The repo's already-shipped Triforce Room already does the equivalent (`TriforceRoomCs.c` drops its `CS_TERMINATOR(81, …)` because 81 warps to Lon Lon Ranch in vanilla).

---

## 2. Cutscene inventory

Every cutscene the port needs, with the source files already vendored. Column key:

- **Where**: scene (and whether we're authoring a new scene or patching a vanilla one).
- **Trigger**: scene-header or manual.
- **Source**: the C file(s) that carry the cutscene data today.

### 2a. Story-beat cutscenes

| # | Cutscene | Where | Trigger | Source today |
|---|---|---|---|---|
| 1 | **Nabooru pan-away** (ends her first meeting in the hideout) | `gerudoway` (vanilla, patched) | Manual, from `Nabooru` actor on dialogue-end | `actors/Nabooru/gerudoway_cs.c` (`NabooruPanAwayCS`) |
| 2 | **Nabooru teaches Full Song of Time** (shehnai + `CS_TEXT_LEARN_SONG`) | `gerudoway` (vanilla, patched) | Manual, from `Nabooru` actor | `actors/Nabooru/gerudoway_cs.c` (`NabooruSecondHalfCS`) |
| 3 | **Desert Colossus pan to Nabooru** (`NbColossusCS`) | `spot11` (vanilla, patched) | Scene-header cutscene via entrance id | `scenes/spot11/spot11cs.c` |
| 4 | **Spot04 CS patch** (Kokiri Forest intro / Lost Woods callback) | `spot04` (vanilla, patched) | Scene-header patch — appends terminator + text to the vanilla file | `scenes/spot04/spot04cspatch.c` |
| 5 | **Running Man intro** (child Link meets the boss) | `spot11` or wherever the race ends (per code-breakdown §2 — spawned by `BossRunningMan`) | Manual, from `BossRunningMan` actor | `actors/BossRunningMan/BossRunningManCutscenes.c` (`BossRunningManIntroCS`) |
| 6 | **Running Man outro** (after beating him, Sages' Charm hand-off) | Same scene as above | Manual, from `BossRunningMan` actor | `actors/BossRunningMan/BossRunningManCutscenes.c` (`BossRunningManOutroCS`) |
| 7 | **Warp to Sacred Realm** (Overture of Sages, Temple-of-Time → Chamber warp) | `tokinoma` (vanilla, patched) | Manual, from `WSRCutscene` actor spawned by `OnOcarinaSongAction` | `actors/WSRCutscene/tokinoma_cs.c` (`WarpToSacredRealmCS`) |
| 8 | **Triforce Room sequence** (29 cam cuts, 10 textboxes, 13 player actions, 2 NPC-action tracks for 40 and the piece sync) | `TriforceRoom` (**new** scene, Besitu slot) | Scene-header `SCENE_CMD_CUTSCENE_DATA` + runtime bump via `TrifoolPercent.cpp` | `scenes/TriforceRoom/TriforceRoom_scene.c` **and** `soh/Enhancements/TrifoolPercent/TriforceRoomCs.c` (vendored, patched copy) |
| 9 | **Finale cutscene** (3,786-frame BotW ending — Zelda/Link, cel shade, 3 BGM tracks, 15 textboxes `0x0E10`–`0x0E21`, 4 NPC action tracks) | `Ending` (**new** scene, Dampé's Hut slot) | Scene-header | `scenes/Ending/Ending_scene.c` (`FinaleCS`) |

### 2b. Staff-roll flyovers

Chained one into the next by a C handler that reads the cutscene's chosen terminator case and sets the next entrance. All ten are text + BGM + camera only — no NPC-action, no player-action — so each file is small and already compiles against SoH's macros.

| Order | Cutscene | Scene (vanilla, patched) | Terminator case id | Source today |
|---|---|---|---|---|
| 1 | **StaffRollFinaleCS** (title + fade from Ending) | `Ending` | — | `scenes/Ending/Ending_scene.c` (`StaffRollFinaleCS`) |
| 2 | **Hyrule Field** | `spot00` | 84 | `scenes/spot00/HyruleFieldFlyover.c` |
| 3 | **Death Mountain Trail** | `spot16` | 85 | `scenes/spot16/DeathMountainTrail.c` |
| 4 | **Gerudo Valley** | `spot09` | 86 | `scenes/spot09/GerudoValleyFlyover.c` |
| 5 | **Deku Tree** | `ydan` | 87 | `scenes/ydan/DekuTreeFlyover.c` |
| 6 | **Zora's Fountain** | `spot08` | 88 | `scenes/spot08/ZoraFountainFlyover.c` |
| 7 | **Zora's River** | `spot03` | 89 | `scenes/spot03/ZoraRiverFlyover.c` |
| 8 | **Kokiri Forest** | `spot04` | 90 | `scenes/spot04/spot04cspatch.c` (uses case `0x5A` = 90) |
| 9 | **Lon Lon Ranch** | `spot20` | 91 | `scenes/spot20/LonLonFlyover.c` |
| 10 | **Haunted Wasteland** | `spot13` | 92 | `scenes/spot13/HauntedWastelandFlyover.c` |
| 11 | **Hyrule Castle** | `spot15` | 93 | `scenes/spot15/CastleFlyover.c` |

Case ids 84–93 are vanilla terminator cases Triforce% reused — some of them unused in vanilla, some reassigned. In SoH our `OnPlayerUpdate` / after-cutscene handler keys off those case ids to pick the next entrance in the chain (see §3 of the engine-capabilities doc on reroute hooks). The vanilla `gEntranceCutsceneTable` entries for 84–93 point at the right vanilla scenes already, so the cutscene itself loads via scene-header; only the *chain-to-next* step is in C.

---

## 3. Asset prerequisites, per cutscene

For each cutscene, the list of **everything that must already be in memory before the cutscene plays** — scene geometry, loaded objects (`SCENE_CMD_SPECIAL_FILES` or `ROOM_CMD_OBJECT_LIST`), registered actors (`ActorDB::Instance->AddEntry`), custom text ids (`CustomMessageManager`), custom BGM, and custom item entries. A missing object renders as a missing-model; a missing actor crashes `Actor_Spawn`; a missing text id falls back to vanilla text; a missing BGM plays silence.

Objects referenced below use the names from `actors/Object list.csv` (slot column 2). Actors use their TrifoolPercent names.

### 3.1 Triforce Room sequence (`gTrifoolTriforceRoomCs`)

**Scene:** `TriforceRoom` (new, Besitu slot, already built).

- **Scene geometry** — Fast64 export of `scenes/TriforceRoom/TriforceRoom.blend` into `.o2r`.
- **Objects loaded by the scene:** `triforcepiece` (4), `triforcechest` (5), `triforcelight` (16), `object_link_child` (21). Triforce Room's `SCENE_CMD_SPECIAL_FILES(0x00, 0x0002)` currently loads `gameplay_field_keep`; the room's object list needs those four slots.
- **Actors (`ActorDB`):** `TriforcePiece` × 3, `TriforceChest` × 1, `TriforceLight` × 1. Already registered in `TrifoolPercent.cpp`. The cutscene drives them via `CS_NPC_ACTION` on two action tracks (ids 40, 2).
- **Custom text ids:** `0x0ACE` ("You got the Triforce!!!"), `0x0E00`–`0x0E07` (goddesses' wish, 8 ids). Already in `CustomMessageManager` per `TrifoolPercent.cpp:214`.
- **Custom BGM:** `PowerOfTheGoddesses.aseq` (ultimately `0x72`). Until the sequence ships, the vendored copy at `soh/Enhancements/TrifoolPercent/TriforceRoomCs.c` plays `NA_BGM_OCARINA_OF_TIME`.
- **Hook wiring:** `OnSceneSpawnActors` for Besitu triggers `cutsceneTrigger`; `OnPlayerUpdate` watches `csCtx.frames` to end the room (vanilla terminator 81 goes to Lon Lon Ranch and is intentionally dropped — the port routes to Ending).

### 3.2 Finale (`FinaleCS`)

**Scene:** `Ending` (new, Dampé's Hut slot).

- **Scene geometry** — Fast64 export of `scenes/Ending/Ending.blend` into `.o2r`.
- **Objects:** `object_link_boy` (adult Link base skeleton, 20), the BotW baked-animation objects (vendored but asset-missing per porting checklist), `SilentPrincess` flowers object, `HolyLight` object (`z_demo_holy` payload), `object_zl2` (adult Zelda, 96). Final list resolved from `Ending_scene.c` object imports once BotW asset files land.
- **Actors:** `BotWLink`, `BotWZelda`, `HolyLight`, `SilentPrincess`, `TwitchMessages` (optional for the sky usernames), re-skinned `z_demo_effect` for Spiritual Stones. The scene's actor list is driven by `FinaleCS`'s four `CS_NPC_ACTION_LIST` tracks (ids 17, 16, 37, 36).
- **Custom text ids:** `0x0E10`–`0x0E18`, `0x0E20`–`0x0E21`.
- **Custom BGM:** `HereTogether1`, `HereTogether2`, `HereTogether3` plus their bank. The cutscene calls `CS_PLAY_BGM` three times at frames 1275, 1820, 3327.
- **Voice SFX:** Ganon/boss SFX-slot redirection for English/Japanese Zelda/Link lines — mentioned in code-breakdown §4.4; a later phase.
- **Hook wiring:** Scene-header runs it on entry. The terminator (case 83, frames 3742–3786) hands off to the staff roll (next entry below).

### 3.3 Chamber-of-Sages sequence

Not vendored as a `CS_` array yet (see §4 Gaps). What it will need when authored:

- **Scene geometry** — Fast64 export of `scenes/ChamberOfSages/ChamberOfSages.blend` into `.o2r` (vanilla `kenjyanoma` slot).
- **Objects:** `cosstaircasesobj` (17), `object_warp1` (72), `object_mjin` (97), `object_mjin_flash` (98), plus the vanilla sage objects for each sage NPC.
- **Actors:** `CoSStaircases`, `CoSSages` (orchestrator). `CoSSages` drives the vanilla sage actors through their lines by setting their action params, not by moving them with `CS_NPC_ACTION`.
- **Custom text ids:** `0x0A10`–`0x0A77` (sage dialogue, including the Sheik/Zelda joke 3-way choice).
- **Custom BGM:** `ChamberOfSages.aseq`.
- **Hook wiring:** `OnSceneSpawnActors` for `kenjyanoma` spawns `CoSStaircases`/`CoSSages` and arms the chamber-exit → Triforce-Room reroute.

### 3.4 Warp to Sacred Realm (`WarpToSacredRealmCS`)

**Scene:** `tokinoma` (vanilla, patched).

- **Objects:** `object_warp1` (72), `object_mjin_flash` (98), `object_toki_objects` (94). Vanilla scene already carries these; nothing to add.
- **Actors:** `WSRCutscene` registered in `ActorDB`.
- **Custom text ids:** None in this cutscene; the "You can't warp here!" string is shown outside the cutscene by `Statics_ShouldAbortWarp`.
- **Custom BGM:** `OvertureOfSagesWarp.aseq`.
- **Hook wiring:** `OnOcarinaSongAction` fires `WSRCutscene`'s spawn in `tokinoma`; the actor sets `csCtx.segment` + `cutsceneTrigger`. Terminator case 82 → Chamber of Sages entrance; our reroute handler rewrites `nextEntranceIndex` to the custom slot (`kenjyanoma`).

### 3.5 Nabooru first meeting (`NabooruPanAwayCS`)

**Scene:** `gerudoway` (vanilla, patched).

- **Objects:** Nabooru's shehnai object (bundled with the actor), vanilla gerudo objects.
- **Actors:** `Nabooru` registered in `ActorDB`, spawned from `OnSceneSpawnActors` for `gerudoway` on the child-Link path (`z_en_ge1` rewrite lets child Link through).
- **Custom text ids:** `0x0B66`.
- **Custom BGM:** None (uses vanilla gerudoway BGM).
- **Hook wiring:** Nabooru's actor update sets `csCtx.segment = &NabooruPanAwayCS` on dialogue-end. Terminator case 79 is vanilla-safe for a pan-and-fade (we double-check it does not reroute to an unintended scene when porting; if it does, drop the terminator and leave the room in C like we do for Triforce Room).

### 3.6 Nabooru second half + Full Song of Time (`NabooruSecondHalfCS`)

**Scene:** `gerudoway` (same patched scene as above).

- **Objects/actors:** Same as 3.5.
- **Custom text ids:** `0x0B68`, `0x0B69`, `0x0B6C`, `0x00D5`.
- **Custom BGM:** `0x003D + 1` (gerudo valley theme) and `0x0018 + 1` (song-learned fanfare). Both vanilla.
- **Learn-song payload:** `CS_TEXT_LEARN_SONG(12, 275, 280, 0xFFFF)` for a slot `12` custom song (`FULL_SONG_OF_TIME` / "Long of Time"), plus `CS_TEXT_LEARN_SONG(25, 290, 295, 0x086D)`. Needs `Statics_ocarina.c` ocarina-note registration ported — covered by `OnOcarinaSongAction` + custom song slot in SoH.
- **Hook wiring:** Fires inside the hideout once the Full-Song-of-Time trigger condition is met.

### 3.7 Desert Colossus pan to Nabooru (`NbColossusCS`)

**Scene:** `spot11` (vanilla, patched).

- **Objects:** Nabooru's shehnai object loaded into `spot11`'s object list.
- **Actors:** `Nabooru` (second spawn, re-rig for the Colossus staging). `z_en_ge2` patched to show the correct dialogue cycle.
- **Custom text ids:** `0x0B67`.
- **Custom BGM:** `0x003F + 1` (desert / spirit temple BGM, vanilla).
- **Hook wiring:** Patched scene-header cutscene triggered on the appropriate entrance id. Terminator case 80.

### 3.8 Running Man intro / outro (`BossRunningManIntroCS`, `BossRunningManOutroCS`)

**Scene:** Spot where the race ends (likely `spot11` based on scene coords; confirm against the romhack reference).

- **Objects:** `bossrunningmanobj` (58).
- **Actors:** `BossRunningMan`.
- **Custom text ids:** `0x0C20`, `0x0C21` (intro); outro ids per actor source.
- **Custom BGM:** `0x001B + 1` (vanilla); outro likely plays `0x001C+1` or `0x002A+1` for the reward beat — confirm against romhack.
- **Hook wiring:** `BossRunningMan`'s state machine sets `csCtx.segment` at race-start and race-win.

### 3.9 Kokiri Forest / Lost Woods patch (`spot04cspatch`)

**Scene:** `spot04` (vanilla, patched).

- **Objects:** `object_oE4` / `object_oE9` (Beta Kokiri heads, 76 / 82).
- **Actors:** `BetaKokiri` already registered.
- **Custom text ids:** `0x0F60`, `0x0F61`.
- **Hook wiring:** Appended to the vanilla spot04 cutscene table rather than fully replacing it. Terminator `0x5A` (= 90) chains to Lon Lon in the staff roll.

### 3.10 Staff-roll flyovers (ten files + `StaffRollFinaleCS`)

**Scenes:** `spot00`, `spot03`, `spot04`, `spot08`, `spot09`, `spot13`, `spot15`, `spot16`, `spot20`, `ydan`, each vanilla and patched.

- **Objects:** Each scene uses its own vanilla object set — no new objects needed, these are camera-only flyovers.
- **Actors:** None new. The flyover doesn't spawn anything; it borrows the already-populated scene.
- **Custom text ids:** `0x0F00`–`0x0F92` and `0x0FA0`+ for the ending title. Register these in `CustomMessageManager` as table entries.
- **Custom BGM:** `StaffRoll.aseq`, `Trailer.aseq`. The first flyover's `CS_PLAY_BGM(0x76 + 1, …)` on `HyruleFieldFlyover` cites a vanilla slot in the source today — map it to the custom sequence at port time.
- **Hook wiring:** Scene-header cutscene attached via `gEntranceCutsceneTable` for each flyover entrance (static table — we can't mutate it, so we instead set the entrance's `cutsceneIndex` in C via `OnTransitionEnd` and let the engine's `Cutscene_HandleEntranceTriggers` pick up the scene-header). The chain between flyovers is a terminator-case-watching handler that sets `nextEntranceIndex`.

---

## 4. Gaps and risks

- **Chamber-of-Sages cutscene data is not vendored.** `scenes/ChamberOfSages/` has no `_scene.c`, only the `.blend` and the `.out.ld`. Fast64-export it before the Chamber-of-Sages phase.
- **BotW mesh/texture assets are missing.** `BotWLink` and `BotWZelda` are in `ActorDB`, but their models were never redistributed by Triforce% ([`docs/porting-checklist.md`](porting-checklist.md) §2). The Finale cutscene plays with placeholder models until replacements land; the cutscene itself is unblocked.
- **Running Man boss skeleton is missing.** Same gap — the actor file is here, the IK skeleton is not. Intro/outro CS data compiles against SoH, but the actor won't render until the asset lands.
- **Custom BGM slots.** `0x72` (`PowerOfTheGoddesses`), `0x73`–`0x75` (`HereTogether1-3`), `0x76` (staff-roll in-vanilla slot used by `HyruleFieldFlyover` — map carefully) and the Chamber-of-Sages / Overture sequences all need SoH `AudioCollection` entries and `.ootrs` packs. Until then, every `CS_PLAY_BGM` with those slots either plays silence or the wrong vanilla track.
- **Entrance-cutscene table (`gEntranceCutsceneTable`) is static.** Any flyover whose entrance id does not already carry a `cutsceneIndex` in vanilla must have one set from code before the entrance resolves. The porting checklist §1 already lists "Entrance cutscene table entry 30 (Triforce Room entry)" as pending — same pattern for each flyover.
- **Fast64's HarbourMasters fork lag.** The scenes were saved with a specific Fast64 commit (vendored or expected to be installed per `tools/fast64/`). Pin the version used, so re-export of `.blend` → C does not shift macro output.
- **Camera-view-angle encoding.** The vendored Triforce Room `_scene.c` uses raw float bit patterns (`0x42960000`). `TriforceRoomCs.c` already shows the fix — rewrite as `float` literals so modern Fast64 output and hand edits are interoperable.

---

## 5. Dependency-ordered build plan, per room

A room is "cutscene-ready" once every row above it is landed. Boxes mirror the format of `docs/porting-checklist.md` so this file can double as the cutscene section of that checklist.

### Phase A — Triforce Room (unblocked, finish Phase 1)

- [x] Scene geometry re-exported from `TriforceRoom.blend` into `.o2r` (Besitu slot)
- [x] Objects loaded: `triforcepiece`, `triforcechest`, `triforcelight`, `object_link_child`
- [x] Actors registered: `TriforcePiece`, `TriforceChest`, `TriforceLight`
- [x] Text ids `0x0ACE`, `0x0E00`–`0x0E07` in `CustomMessageManager`
- [x] Cutscene data in `TriforceRoomCs.c` (vendored, patched copy)
- [ ] Custom BGM `PowerOfTheGoddesses.aseq` — currently falls back to Ocarina of Time
- [ ] After-cutscene reroute to Ending (replaces the dropped terminator 81)

### Phase B — Sacred Realm (unblocks Chamber + its exit cutscene)

- [ ] Fast64-export `scenes/ChamberOfSages/ChamberOfSages.blend` → `_scene.c` with its cutscene
- [ ] Objects `cosstaircasesobj`, `object_warp1`, `object_mjin`, `object_mjin_flash`, sage objects
- [ ] Actors registered: `CoSStaircases`, `CoSSages`
- [ ] Text ids `0x0A10`–`0x0A77` registered
- [ ] Custom BGM `ChamberOfSages.aseq`
- [ ] Chamber-of-Sages scene header runs the chamber cutscene on entry
- [ ] On chamber-exit, reroute to Triforce Room (`OnTransitionEnd`)
- [ ] `WSRCutscene` actor spawning hook (`OnOcarinaSongAction`) in `tokinoma`
- [ ] `tokinoma` room patch adds a spawn point for `WSRCutscene`
- [ ] Terminator 82 → Chamber of Sages reroute

### Phase C — Unicorn Fountain (needed for the Overture song flow)

- [ ] Fast64-export `scenes/UnicornFountain/UnicornFountain.blend` → `.o2r` (Granny's Potion Shop slot)
- [ ] Objects for `BetaGreatFairy` + `object_mjin` (23, the ocarina pedestal scale variant)
- [ ] Actors registered: `BetaGreatFairy`, `z_bg_mjin` scale variant
- [ ] Scene cutscene for Great Fairy rise/fall (Fast64-authored; not yet in repo)
- [ ] Text ids for the fairy's teaching
- [ ] Custom BGM `OvertureOfSages.aseq`
- [ ] Zora's Domain → Unicorn Fountain reroute wired in `Statics_SetUpRouting` port

### Phase D — Gerudo path (child-Link entrance + Nabooru)

- [ ] `z_en_ge1` rewrite lets child Link into `gerudoway`
- [ ] Objects for `Nabooru` (shehnai) loaded into `gerudoway` object list
- [ ] Actor registered: `Nabooru`
- [ ] Text ids `0x0B50`–`0x0B53` (guards), `0x0B66`–`0x0B6C`, `0x00D5` registered
- [ ] `NabooruPanAwayCS` fires on first-meeting dialogue end
- [ ] `NabooruSecondHalfCS` fires on second-meeting trigger; `CS_TEXT_LEARN_SONG` teaches Full Song of Time
- [ ] `spot11` patched scene header carries `NbColossusCS`
- [ ] Shehnai object loaded in `spot11` object list

### Phase E — Running Man path (Power beat)

- [ ] Running Man boss skeleton asset located or substituted
- [ ] Object `bossrunningmanobj` built into `.o2r`
- [ ] Actor registered: `BossRunningMan`
- [ ] Text ids `0x0C20`, `0x0C21`, outro ids
- [ ] `BossRunningManIntroCS` fires on race-start
- [ ] `BossRunningManOutroCS` fires on race-win, grants Sages' Charm (`GetItemEntry` via `ItemTableManager`)

### Phase F — Ending (the big one)

- [ ] BotW Link / Zelda meshes located or substituted
- [ ] Fast64-export `scenes/Ending/Ending.blend` → `.o2r` (Dampé's Hut slot)
- [ ] Objects: BotW Link/Zelda, `SilentPrincess`, `HolyLight`, `object_zl2`
- [ ] Actors registered: `BotWLink`, `BotWZelda`, `HolyLight`, `SilentPrincess`, `TwitchMessages`
- [ ] Text ids `0x0E10`–`0x0E18`, `0x0E20`–`0x0E21`
- [ ] Custom BGM `HereTogether1-3`
- [ ] Voice SFX slot redirection (English + Japanese)
- [ ] `FinaleCS` already in `Ending_scene.c` — only compiles once objects/actors are in
- [ ] Terminator case 83 → Staff Roll chain

### Phase G — Staff Roll chain

- [ ] Register text ids `0x0F00`–`0x0F92` and `0x0FA0`+
- [ ] Custom BGM `StaffRoll.aseq`, `Trailer.aseq`
- [ ] `OnTransitionEnd` handler sets each flyover entrance's `cutsceneIndex` to its scene-header cutscene (since `gEntranceCutsceneTable` is static)
- [ ] Chain terminators 84 → 93 in a single reroute handler
- [ ] `StaffRollFinaleCS` fires at the end of the chain, returns to title

---

## 6. Recommendations in one line

Author anything with camera/action tracks in **Fast64 (HarbourMasters fork)** against the existing `.blend`, hand-write anything that is just text/BGM/transition, keep `soh/include/z64cutscene_commands.h` open as the macro reference, build the **Triforce% romhack ROM once** and have it running next to SoH as the visual oracle, and port rooms in the order Phase A → Phase B → ... → Phase G so each cutscene's prerequisites are already in memory by the time it fires.
