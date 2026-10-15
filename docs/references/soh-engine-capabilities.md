# SoH engine capabilities: what TrifoolPercent can reuse

A short map of what Ship of Harkinian (SoH) handles natively, so TrifoolPercent only writes new C/C++ where the engine genuinely requires it. Written 2026-10-15 against Shipwright `develop` and cross-checked against the TrifoolPercent module already in this repo (`soh/Enhancements/TrifoolPercent/`).

**TL;DR.** Dialog boxes, item-get sequences, cutscene playback and loading-zone transitions are all already in SoH's native OoT engine. The engine gives us hooks and manager classes to *inject our own data* (new text ids, new `GetItemEntry`, new cutscenes, re-targeted entrances) without writing custom actors or custom scene commands. The only places that unavoidably want new C/C++ are: new actor behaviour (`ActorDB` + a C file), rewiring entrances in response to game state (`OnTransitionEnd` hook), and anything that reaches into fields the engine doesn't expose (none of the four systems below).

---

## 1. Custom dialog boxes

**Native. No new C needed beyond a one-time hook registration.**

SoH keeps the vanilla OoT message system (`Message_OpenText(play, textId)`, textbox styles, control bytes `\x01` newline, `\x02` end, `\x05xx` color, `\x06xx` shift, `\x0E` fade, `\x14` speed, `\x1A` unskippable, `\x1C` 3-way choice, etc.). It adds two tools on top:

- **`CustomMessageManager`** (`soh/Enhancements/custom-message/CustomMessageManager.h`): named message tables with English/French/German strings, auto-formatting, item-icon pages and `Replace`/`InsertNames`/`InsertNumber` substitution. API: `AddCustomMessageTable(tableID)`, `CreateMessage(tableID, textID, CustomMessage)`, `CreateGetItemMessage(tableID, giid, iid, msg)`, `RetrieveMessage(tableID, textID, format)`. A `CustomMessage` carries `TEXTBOX_TYPE_*` and `TEXTBOX_POS_*` and exposes `LoadIntoFont()` to push raw bytes straight into the game's font buffer.
- **`OnOpenText(uint16_t* textId, bool* loadFromMessageTable)`** hook (`GameInteractor_HookTable.h`): fires every time the engine is about to open a textbox. The handler can rewrite `*textId`, set `*loadFromMessageTable = false` and `LoadIntoFont()` to supply the string itself. This is exactly how TrifoolPercent already serves `0x0ACE` and the goddesses' `0x0E00`–`0x0E07` in `TrifoolPercent.cpp:214`.

**Triggering a textbox** uses the vanilla paths — nothing SoH-specific:

- From a chest: fill `GetItemEntry.textId` on the item (see §2); `En_Box` passes it through `Actor_OfferGetItemNearby`.
- From an NPC: call `Message_StartTextbox(play, textId, &actor->actor)` in the actor's `Update`.
- On scene entry: in an `OnSceneSpawnActors` or `OnTransitionEnd` handler, call `Message_StartTextbox(gPlayState, textId, NULL)` once the fade-in is done.
- From a cutscene: `CS_TEXT_DISPLAY_TEXTBOX(textId, startFrame, endFrame, type, altText1, altText2)` — see §3.

**What this means for the port.** Every piece of Triforce% dialogue (`0x0B10`–`0x0B16` Beta Kokiri, `0x0B50`–`0x0B53` Gerudo guards, `0x0A10`–`0x0A77` Chamber of Sages, item-get names) is a `CustomMessageManager` table entry. No new C. Triforce%'s own `statics/text.c` ROM-patching loader is dead weight: it only existed to inject the table into an unmodified cartridge. We register one `OnOpenText` filter that routes TrifoolPercent ids to our table and leaves everything else alone, pattern already proven in `LoadTriforceRoomMessage`.

## 2. Item-get sequences

**Native. No new actor needed; use `En_Box` and `ItemTableManager`.**

SoH ships `ItemTableManager` (`soh/Enhancements/item-tables/ItemTableManager.h`) and the `MOD_NONE`/mod-id lookup is wired through `ItemTable_RetrieveEntry` inside the SoH `En_Box` (`soh/src/overlays/actors/ovl_En_Box/z_en_box.c`). API: `AddItemTable(tableID)`, `AddItemEntry(tableID, getItemID, GetItemEntry)`, `RetrieveItemEntry(...)`.

A `GetItemEntry` already carries everything a vanilla get-item needs: `itemId`, `field`, `gi` (`GetItemID`), `textId`, `objectId`, `modIndex`, `getItemId`, `getItemCategory`, and the `GID_*` draw id (plus a `GetItemEntry_Draw` function pointer for a custom draw).

**Pipeline for a custom "Butterfly / Sages' Charm / Triforce" get:**

1. Load the custom object (model + icon) through the `.o2r` archive at a resource path.
2. In `OnGameStateMainStart` (one-shot init), build a `GetItemEntry {.modIndex = MOD_RANDOMIZER or a TrifoolPercent id, .getItemId = TRIFOOL_GI_BUTTERFLY, .itemId = ITEM_BUTTERFLY, .textId = 0x009D, .objectId = OBJECT_GI_BUTTERFLY, .gid = GID_BUTTERFLY, .getItemCategory = ITEM_CATEGORY_MAJOR}` and call `ItemTableManager::Instance->AddItemEntry(TRIFOOL_ITEM_TABLE, TRIFOOL_GI_BUTTERFLY, entry)`.
3. Create the text id with `CustomMessageManager::Instance->CreateGetItemMessage(table, giid, iid, CustomMessage(...))` — this stamps the `ITEM_OBTAINED` icon header automatically.
4. In the scene's room `.o2r`, put an `ACTOR_EN_BOX` with `params` packed as `(treasureFlag) | (getItemId << 5) | (chestType << 12)`. En_Box calls `ItemTable_RetrieveEntry(modIndex, getItemId)` and runs the vanilla "chest opens, Link holds item overhead, textbox" sequence. **The item-get fanfare, pose, text and freeze-on-player are all free.** See `z_en_box.c` lines handling `ItemTable_RetrieveEntry(MOD_NONE, ...)`.
5. For non-chest sources (NPC trade, pickup actor), call `GiveItemEntryFromActor(&thisActor, play, entry, 50.0f, 10.0f)` with the same `GetItemEntry`.

The 7-bit `params` field for the chest's `getItemId` caps at 128. If TrifoolPercent exceeds that in one scene (unlikely — Triforce% itself used fewer), we either reuse ids across scenes or spawn the chest from `OnSceneSpawnActors` with a wider id, which also side-steps the room-list limit. Triforce%'s `0x4C` custom draw function for the Sages' Charm maps onto `GetItemEntry_Draw`.

**What this means for the port.** Triforce%'s custom item-get machinery (Butterfly moving into the child trade slot, the Sages' Charm reusing the Stone of Agony slot, "You got the Triforce!!!") is all `ItemTableManager` + `CustomMessageManager` + a stock `En_Box`. No new actor code for the chest itself. The only exception is the Triforce Room's "piece rises → combine → sink → absorb", which is a *cutscene*, not an item-get (§3), and is already a `TriforcePiece` state-machine actor in this repo.

## 3. Cutscenes

**Native, with one caveat.** SoH plays vanilla OoT cutscene scripts byte-for-byte: `Cutscene_ProcessCommands` in `soh/src/code/z_demo.c` walks the same `CS_*` command table, and the macro header (`soh/include/z64cutscene_commands.h`) ships `CS_BEGIN_CUTSCENE`, `CS_CAM_EYE_LIST`/`CS_CAM_AT_LIST` (and the newer `CS_CAM_POS_*`/`CS_CAM_FOCUS_POINT_*` aliases), `CS_NPC_ACTION`, `CS_PLAYER_ACTION`, `CS_LIGHTING`, `CS_TEXT_DISPLAY_TEXTBOX`, `CS_TEXT_LEARN_SONG`, `CS_PLAY_BGM` / `CS_STOP_BGM` / `CS_FADE_BGM`, `CS_TIME`, `CS_SCENE_TRANS_FX`, `CS_MISC`, `CS_TERMINATOR`, `CS_END` and the historical `CS_CMD_07..09`/`CS_UNK_DATA`.

**Caveat (not a blocker).** The SoH dispatcher has a few branches that are not pure vanilla:

- `GameInteractor_Should` hook calls on text cases `0x3C`–`0x41`, `0x72`, `0x80`–`0x82` (skippable text, randomizer rerouting).
- A `BetterFarore` CVar branch in case `8`.
- A `CreditsFix` delay in terminator cases `55`–`78` to patch Nintendo's original off-by-one on the credits reel.

None of these are new *commands* we have to author; they are tweaks to existing ones. Our cutscenes compile with the same macros Fast64 and `z64hdr` emit.

**Two ways to play a cutscene — both already supported:**

1. **Scene cutscene list.** Set the scene header's cutscene list (`SCENE_CMD_CUTSCENE_DATA`) in the room `.o2r`, make the scene the current entrance's `sceneLoadFlag`/`cutsceneIndex`, and the engine runs it on entry. The vanilla `Cutscene_HandleEntranceTriggers` path still fires, consulting `gEntranceCutsceneTable`.
2. **Manual trigger from C.** As `TrifoolPercent.cpp:103` already does:
   ```c
   gPlayState->csCtx.segment = <ptr to CutsceneData[]>;
   gSaveContext.cutsceneTrigger = 1;
   ```
   `Cutscene_SetSegment` handles segmented or virtual pointers; a resource loaded from `.o2r` works too (`Cutscene_ProcessCommands` detects the `OTR` signature and calls `ResourceMgr_LoadCSByName`).

**Terminator.** `CS_TERMINATOR(destination, startFrame, endFrame)` sets the next entrance and fade style. SoH respects this. Case `113` is the only hard-coded override (switches to `gTowerBarrierCs` for the Ganon's Tower collapse); we do not reuse that case.

**What this means for the port.** All of Triforce%'s cutscenes (Nabooru teaching, Gerudoway intros, Spot11 pan, Running Man intro, Triforce Room, Chamber of Sages, Ending flyovers, staff roll) can be written as plain OoT cutscene data, exported by Fast64 or hand-written with the SoH macros, and played either from the scene header or from an `OnSceneSpawnActors`/`OnTransitionEnd` C handler. **No cutscene command needs to be invented or extended.** The piece-rises-and-combines sequence in the Triforce Room is the pattern: cutscene data for camera, lights, music, text; a small actor for anything the cutscene can't express (synchronized particle rings, light-column scaling), spawned from the cutscene via `CS_NPC_ACTION`.

## 4. Scene entry triggers / loading zones

**Partially native.** Vanilla scenes already carry their own exit lists and spawn lists; loading zones in a room (`SCENE_CMD_EXIT_LIST` + the room's door/collision polys with exit indices) point into `gEntranceTable`, and the engine resolves them with no custom code. What SoH does *not* let an `.o2r` do is rewrite the entrance table — `gEntranceTable` is static C data. So:

- **A loading zone in a modified scene (room `.o2r`) can point at any existing `ENTR_*` value.** If that value already resolves to a vanilla scene, the engine takes you there with no C-side help. This is enough for every loading zone that still goes to a vanilla destination.
- **A loading zone that must point at a custom (hijacked) scene is a two-step dance.** Pick an entrance that already targets the hijacked scene slot (e.g. `ENTR_BESITU_0` for the Besitu debug slot used by the Triforce Room POC) and set that index in the exit list. If no suitable vanilla entrance exists, intercept with `OnTransitionEnd(int16_t sceneNum)` or `OnPlayerUpdate` and reroute by setting `gPlayState->nextEntranceIndex` / `transitionTrigger = TRANS_TRIGGER_START` / `transitionType = TRANS_TYPE_FADE_BLACK_FAST`. The repo's own test loading zone (`EnterTriforceRoomFromTestEntrance` in `TrifoolPercent.cpp`) is this pattern.
- **Entrance-cutscene table (`gEntranceCutsceneTable`).** Also static. To attach a cutscene to a custom entry, trigger it from code in `OnSceneSpawnActors` — exactly how `SpawnTriforceRoom` kicks the Triforce Room cutscene.

**Hooks available for entrance work** (from `GameInteractor_HookTable.h`): `OnSceneInit`, `AfterSceneCommands`, `OnSceneSpawnActors`, `OnTransitionEnd`, `OnPlayerUpdate`, `OnWarpSongLeave`, `OnLoadGame`.

**What this means for the port.** The scene-routing patch in Triforce%'s `statics/scene.c` (`Statics_SetUpRouting`) does **not** port to a static table; it ports to **one C module** that registers `OnTransitionEnd` and `OnPlayerUpdate` hooks and overrides `nextEntranceIndex` for the handful of edges Triforce% rewires (Zora's Domain → Unicorn Fountain, Chamber of Sages → Triforce Room, Triforce Room → Ending, each staff-roll flyover → the next). The exit lists inside our modified vanilla scenes (`.o2r`) just need to carry *an* entrance index that we then rewrite in C. This is `<20` lines of code per routed edge and reuses the pattern already in `TrifoolPercent.cpp`.

---

## What genuinely needs new C/C++

Only two things, by the time §1–§4 are settled:

1. **New actor behaviour.** Any Triforce% actor that was more than data (BetaKokiri, Nabooru, BossRunningMan, BetaGreatFairy, WSRCutscene, CoSStaircases, CoSSages, TriforcePiece/Chest/Light, BotWLink, BotWZelda, HolyLight, SilentPrincess, TwitchMessages, and rewrites of z_en_skj / z_en_ge1 / z_en_mm3 / z_bg_spot07_taki / z_en_owl) is a `.c` file registered with `ActorDB::Instance->AddEntry(...)` (pattern in `AddActor` in `TrifoolPercent.cpp`). SoH's `ActorDB` assigns the id at runtime; actors are spawned from `OnSceneSpawnActors` rather than from the room's actor list.
2. **Vanilla-actor hooks.** Triforce% shipped MIPS diffs against `z_player`, `z_kaleido_scope`, `z_en_xc`, `z_demo_effect`, `z_bg_toki_swd`, `z_en_ge2`, `z_en_girla`, `z_oceff_wipe`, `z_bg_mjin`. In SoH these become `GameInteractor::OnVanillaBehavior` filters (via `REGISTER_VB_SHOULD`) or small direct edits to the actor source in our fork. The engine already carries the `VB_*` ids for most of the touch points Triforce% needed (chest open, treasure flag, OnePoint cutscenes, ice effect; player sword/pedestal; kaleido C-button edits).

Everything else — dialog text, item entries, cutscene playback, scene transitions, entrance-cutscenes, title cards (`TitleCard_InitPlaceName`), save flags (`gSaveContext.eventChkInf`, `infTable`, `itemGetInf`), magic/double magic grants, Bunny Hood speed/jump (already a SoH enhancement), Ocarina time travel without the Master Sword (already a SoH enhancement) — rides on code that is already in the engine.

## Hook cheatsheet (what to reach for)

| Need | Hook / API |
|---|---|
| Add a text id with English/FR/DE | `CustomMessageManager::Instance->CreateMessage(tableID, textID, msg)` + `OnOpenText` routing |
| Add a get-item (icon, model, text) | `ItemTableManager::Instance->AddItemEntry` + `CustomMessageManager::CreateGetItemMessage` |
| Fire a textbox from code | `Message_StartTextbox(play, textId, actor_or_NULL)` |
| Run a cutscene from code | set `csCtx.segment` + `gSaveContext.cutsceneTrigger = 1` |
| Attach a cutscene to a scene entry | `OnSceneSpawnActors` + the above (static `gEntranceCutsceneTable` is read-only) |
| Reroute an entrance | `OnTransitionEnd` / `OnPlayerUpdate` + `play->nextEntranceIndex`, `transitionTrigger`, `transitionType` |
| Spawn a custom actor | `ActorDB::Instance->AddEntry(...)` once, `Actor_Spawn` from `OnSceneSpawnActors` |
| Override a vanilla behaviour | `REGISTER_VB_SHOULD(VB_*, lambda)` |
| Set event flags, give items from script | `Flags_SetEventChkInf`, `Item_Give`, `gSaveContext.*` directly |
| Scene arrival title card | `TitleCard_InitPlaceName(play, &play->actorCtx.titleCtx, texture, ...)` |

## Sources

- `soh/soh/Enhancements/custom-message/CustomMessageManager.h` (SoH `develop`)
- `soh/soh/Enhancements/item-tables/ItemTableManager.h` (SoH `develop`)
- `soh/soh/Enhancements/game-interactor/GameInteractor_HookTable.h` (SoH `develop`)
- `soh/src/overlays/actors/ovl_En_Box/z_en_box.c` (SoH `develop`)
- `soh/src/code/z_demo.c` (SoH `develop`, `Cutscene_ProcessCommands`, `cutsceneTrigger`)
- `soh/include/z64cutscene_commands.h` (SoH `develop`, `CS_*` macros)
- `soh/src/code/z_scene.c` (SoH `develop`, scene-command dispatch)
- This repo's own `soh/Enhancements/TrifoolPercent/TrifoolPercent.cpp` — a working reference for `ActorDB`, `OnOpenText`, `OnSceneSpawnActors`, `OnPlayerUpdate` and the manual-cutscene trigger
- Triforce% source breakdown: [`docs/references/triforcepercent/code-breakdown.md`](triforcepercent/code-breakdown.md)
