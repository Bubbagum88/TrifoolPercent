# TrifoolPercent Porting Checklist

Every mechanic, scene, actor, and asset from the original [Triforce%](https://github.com/triforce-percent/triforce-percent) run that needs to land in TrifoolPercent. Grouped by category, with each item an unchecked box.

Sourced from [docs/references/triforcepercent/code-breakdown.md](references/triforcepercent/code-breakdown.md) and the upstream `run/triforcepercent_data.run` manifest. Items marked **(already in SoH)** can be gated on a TrifoolPercent flag rather than reimplemented. Items marked **(asset missing)** are not in the upstream repo (BotW/derivative) and need an original replacement.

---

## 0. Before you start a scene — build the reference romhack

The upstream repo builds **two** things from the same source: the live ACE run and a plain romhack ROM (`rom-setup/romhack`). The romhack ROM is just the ACE run with every `PATCH/ACTOR/OBJECT/SCENE/STATICDATA` entry from `run/triforcepercent_data.run` pre-applied, so it plays in any N64 emulator. Emulators cannot run the live ACE version — the romhack build is the only way to see the quest end-to-end outside of TASBot.

Set this up once, locally, before porting the first scene, and keep an emulator open next to the SoH build throughout the port:

- Build target: `rom-setup/romhack` in [triforce-percent/triforce-percent](https://github.com/triforce-percent/triforce-percent) (needs Linux/WSL, the glankk N64 toolchain, and your own OoT 1.0U compressed ROM — no Nintendo data is distributed).
- Use it as the **authoritative visual/behavioural reference** for every scene, cutscene, actor, dialogue flow, and audio cue you port. If a scene-header or actor-list diff is ambiguous in the source, dumping the built ROM resolves it.
- `statics/statics.c` → `Statics_TestShortcuts` has commented-out warps to each story beat; mirror these with SoH's debug warp to jump directly to whatever you're currently porting.
- Do **not** ship or redistribute the built ROM — it contains vanilla OoT data. Local reference use only.

See code-breakdown §7 for full build notes.

---

## 1. Scenes

New scenes (Fast64 `.blend` sources upstream, re-export to `.o2r`):

- [x] Unicorn Fountain (lives in Granny's Potion Shop scene slot)
- [x] Chamber of Sages (replaces vanilla `kenjyanoma`)
- [x] Triforce Room (lives in Goron Shop scene slot) — POC underway in Besitu slot
- [x] Ending (lives in Dampé's Hut scene slot)

Vanilla scene / room patches (reapply as resource replacements or `OnSceneSpawnActors`):

- [ ] Lost Woods rooms 5 and 7 (secret-path trigger wiring)
- [x] Hyrule Market Bazaar (shop shelf swap for Butterfly)
- [x] Gerudo Hideout / `gerudoway` (child-Link entry, Nabooru cutscenes)
- [x] Desert Colossus / `spot11` (Nabooru pan cutscene)
- [x] Zora's Domain (ice-melt, exit reroute to Unicorn Fountain)
- [ ] Kakariko (Granny's door turnaround)
- [x] Temple of Time / `tokinoma` room patch (WSR warp spawn)
- [ ] Staff-roll flyovers: Hyrule Field
- [ ] Staff-roll flyovers: Death Mountain Trail
- [ ] Staff-roll flyovers: Gerudo Valley
- [ ] Staff-roll flyovers: Deku Tree
- [ ] Staff-roll flyovers: Zora's Fountain
- [ ] Staff-roll flyovers: Zora's River
- [ ] Staff-roll flyovers: Kokiri Forest
- [ ] Staff-roll flyovers: Lon Lon Ranch
- [ ] Staff-roll flyovers: Wasteland
- [ ] Staff-roll flyovers: Hyrule Castle
- [x] Entrance-table rewiring (`Statics_SetUpRouting`): Zora's Domain → Unicorn Fountain (`0x0560`), Chamber → Triforce Room, Triforce Room → Ending, plus the staff-roll chain
- [ ] Entrance cutscene table entry 30 (Triforce Room entry)
- [ ] Cutscene terminator chain in `statics/scene.c` (sequence staff-roll flyovers)

---

## 2. Custom actors (new code — needs ActorDB registration + object in `.o2r`)

- [x] `BetaKokiri` (replaces actor slot 49; reuses beta Kokiri head/body)
- [x] `Nabooru` (new 536-line actor, shehnai model, 3 anims, 2 in-engine cutscenes)
- [x] `BossRunningMan` (1,679 lines, custom IK skeleton, AI, intro cutscene) — **(asset missing: boss skeleton)**
- [x] `BetaGreatFairy` (rise/fall, sparkle particles, scene cutscene)
- [x] `WSRCutscene` (Warp-Song-Replacement Temple of Time cutscene actor)
- [x] `CoSStaircases` (Rauru/Saria/Sheik staircases, collision, Triforce door, fade wall, hallway)
- [x] `CoSSages` (drives vanilla sage NPCs, fade-in particles, dialogue 0x0A10–0x0A77)
- [x] `TriforcePiece` (3 pieces, state machine: rise → combine → sink → absorb)
- [x] `TriforceChest` (beta chest model + animation)
- [x] `TriforceLight`
- [x] `BotWLink` (baked animations, hair/tunic physics, cel shading) — **(asset missing: mesh/textures)**
- [x] `BotWZelda` (baked animations, hair/tunic physics, cel shading) — **(asset missing: mesh/textures)**
- [x] `HolyLight` / `z_demo_holy`
- [x] `SilentPrincess` (flowers)
- [x] `TwitchMessages` (live chat usernames/emotes rendered in sky)

Rewritten vanilla actors (full replacement — fork the source or spawn custom variant):

- [x] `z_en_skj` (Skull Kid: recognises Magic Powder, upgrades Bunny Hood / Gerudo Mask)
- [x] `z_en_ge1` (Gerudo guard, lets child Link through)
- [x] `z_en_mm3` (Running Man race)
- [x] `z_bg_spot07_taki` (Zora's Domain ice-melt, by z64me)
- [x] `z_en_owl` (always-kill: skip Kaepora dialogue)

Patched vanilla actors (upstream ships as MIPS diffs — reimplement as `VB_*` hooks / small edits):

- [x] `z_player` (always-child treatment for sword/pedestal, Din's Fire on C, Bunny Hood run/jump)
- [x] `z_kaleido_scope` (Medallions to C buttons, magic-bar patches)
- [x] `z_en_xc` (Sheik treats Link as child)
- [x] `z_demo_effect` (Spiritual Stones treat Link as child)
- [x] `z_bg_toki_swd` (Master Sword pedestal: no-op pull-sword)
- [x] `z_en_ge2` (Gerudo guard dialogue cycle 0x0B50–0x0B53)
- [x] `z_en_girla` (shop item: Butterfly in Bazaar slot 3)
- [x] `z_oceff_wipe` (ocarina / warp FX)
- [x] `z_bg_mjin` (Pedestal of the Ocarina: scale variant, actor slot 23)

---

## 3. Mechanics / engine glue (`statics/`)

Quest state and flow:

- [x] `Statics_SetGameState` — set event flags (Epona, DMT not on fire, carpenters, masks sold), grant magic + double magic, give adult B = Biggoron Sword, `naviTimer = 1`
- [ ] Beta inventory editor on pause (`L` → `pauseCtx.debugState = 1`)
- [x] `Statics_LostWoods` — N→W→S→W path tracking, set `BETAKOKIRI_SPAWNED`
- [x] `Statics_PatchShop` — Bazaar shelf-3 swap to Butterfly after text `0x009C`
- [x] `Statics_MoveOddPotionToChild` — Butterfly reward → child trade slot
- [ ] Custom save flags (prefer SoH `SaveManager` custom section over borrowing `infTable` / `itemGetInf` bits)
- [x] `Statics_SetUpRouting` — runtime entrance-table rewiring
- [x] `Statics_TestShortcuts` — debug warps to every story beat (mirror with SoH debug warp)
- [ ] Per-frame quest hook (`Statics_Player_Update` → SoH `OnPlayerUpdate`)

Items and inventory:

- [ ] Butterfly item (reuses `ITEM_BIG_POE` slot, new icon/name/model)
- [ ] Magic Powder trade (Butterfly → Powder)
- [ ] Sages' Charm item (reuses Stone of Agony GI slot, new 24px icon)
- [ ] Medallions equippable on C buttons (gated on Sages' Charm)
- [ ] Fire Medallion on C performs Din's Fire
- [ ] Magic-bar patches for medallion use

Movement and traversal:

- [ ] Bunny Hood: 1.5× run, 1.25× jump — gate SoH `BunnyHood.cpp` on powder-applied flag **(already in SoH)**
- [ ] Working Gerudo Mask lets child Link past the guard
- [x] Zora's River → Zora's Domain warp (Sages' Charm feature)

Ocarina / songs:

- [x] Chromatic ocarina (flat/sharp with D-pad modifiers)
- [x] Shehnai instrument timbre during Nabooru teaching
- [x] "Long of Time" / full Song of Time definition (`statics/ocarina.c`)
- [x] `Statics_TimeTravel` — flip `linkAgeOnLoad`, reload entrance, no Master Sword required **(already in SoH: `OcarinaTimeTravel.cpp`)**
- [x] Overture of Sages (new song, note data + table extension)
- [x] `Statics_ShouldAbortWarp` — Overture hijacks warp-song path in Temple of Time
- [ ] "You can't warp here!" message elsewhere

Running Man race:

- [x] Timer branch in `Statics_TimeTravel`
- [x] `Statics_KeepTimerNegative`
- [ ] Spawn `ACTOR_EN_EX_RUPPY` (exploding giant magenta rupee)

Rendering / animation:

- [ ] Custom Link animations: LookToMeditate
- [ ] Custom Link animations: Meditate
- [ ] Custom Link animations: ReachUp
- [ ] SLERP morph on patched SkelAnime (optional polish)
- [x] `hairphys.c` — hair / tunic physics for BotW actors
- [ ] F3DZEX cel-shading microcode (BotW Link/Zelda) — needs libultraship / shader-side approach, no RSP microcode in SoH
- [ ] Chamber of Sages scene draw config: animated water + light rays fading near player
- [ ] Spawn trigger for Arwing (vanilla `En_Clear_Tag`, L+D-Down) **(already in SoH)**

Audio:

- [ ] SFX redirection table (Ganon/boss SFX slots repurposed for voice)
- [ ] Voice playback path (new sample resources + play code path)

---

## 4. Cutscenes

- [ ] Nabooru intro / teaching cutscenes (`gerudoway_cs.c`)
- [x] Desert Colossus pan cutscene (`spot11`)
- [ ] Running Man boss intro cutscene
- [ ] Beta Great Fairy scene cutscene (Unicorn Fountain)
- [x] WSR Temple of Time cutscene (Overture played at pedestal)
- [ ] Chamber of Sages cutscene (sage fade-in, dialogue, Zelda/Sheik reveal)
- [ ] Triforce Room entrance cutscene (`gEntranceCutsceneTable[30]`)
- [ ] Triforce Room sequence (pieces rise → combine → sink → absorb)
- [ ] Goddesses' wish three-choice sequence (`0x0E00`–`0x0E07`)
- [ ] Ending `FinaleCS` (BotW Link + Zelda, Silent Princess, HolyLight)
- [ ] Staff-roll flyover chain terminators (`statics/scene.c`)

---

## 5. Text / messages (`text/messages.c`)

Port via `CustomMessageManager::CreateMessage(tableID, textID, msg)`. Standard OoT control codes (`BOX_BREAK`, `TWO_CHOICE`, `EVENT`, `FADE`, `QUICKTEXT`) port directly.

- [x] Beta Kokiri dialogue (`0x0B10`–`0x0B16`)
- [x] Bazaar Butterfly text (`0x009C`)
- [x] Gerudo guard rotation (`0x0B50`–`0x0B53`)
- [x] Chamber of Sages dialogue (`0x0A10`–`0x0A77`, incl. boy/girl choice)
- [x] "You got the Triforce!!!" (`0x0ACE`, uses beta Triforce icon)
- [x] Goddesses' wish three-choice (`0x0E00`–`0x0E07`)
- [x] Staff Roll text (`0x0F00`+)
- [x] "You can't warp here!" fallback for non-ToT Overture plays
- [x] Butterfly / Sages' Charm / Overture item-get text
- [x] Nabooru / Skull Kid / Running Man quest dialogue
- [x] Beta Great Fairy teaching dialogue

---

## 6. Audio (sequences + banks + samples)

Music sequences (11 `.aseq` upstream; convert to SoH sequence format):

- [x] `OvertureOfSages.aseq`
- [x] `OvertureOfSagesWarp.aseq`
- [x] `ChamberOfSages.aseq` (+ custom bank)
- [x] `triforce.aseq`
- [x] `PowerOfTheGoddesses.aseq`
- [x] `HereTogether1.aseq` (+ bank)
- [x] `HereTogether2.aseq`
- [x] `HereTogether3.aseq`
- [x] `StaffRoll.aseq`
- [x] `Trailer.aseq`
- [ ] 11th sequence (verify slot vs. upstream manifest)

Instrument banks:

- [x] Shehnai bank (Nabooru ocarina)
- [x] Chamber of Sages bank
- [x] HereTogether bank

Voice / SFX samples (22 VADPCM):

- [x] BotW Link voice clips (English)
- [ ] BotW Link voice clips (Japanese)
- [x] BotW Zelda voice clips (English)
- [ ] BotW Zelda voice clips (Japanese)
- [ ] Remaining ambience / SFX samples (crosscheck upstream `voice/`)

---

## 7. Models / objects / textures

Objects (upstream has these):

- [x] Triforce piece object (`.zobj`)
- [x] Triforce chest object (beta model + animation)
- [x] Triforce light object
- [x] Sages' Charm GI model
- [x] Butterfly GI model (`object_gi_butterfly`, beta item mesh)
- [x] Nabooru shehnai object
- [x] CoS staircases geometry
- [ ] Unicorn Fountain scene geometry (Fast64 `.blend`)
- [ ] Chamber of Sages scene geometry
- [ ] Triforce Room scene geometry
- [ ] Ending scene geometry (~4 MB `.blend`)

Textures / UI:

- [x] Butterfly item icon + name
- [x] Sages' Charm icon (24px)
- [x] Beta Triforce item icon (for `0x0ACE`)
- [ ] Title cards (`title.ia8.png`) for each new scene
- [x] Twitch emotes
- [x] Twitch badges

Assets NOT in upstream — originals required:

- [ ] BotW Link mesh + textures + eye `.inc`
- [ ] BotW Zelda mesh + textures + eye `.inc`
- [ ] Running Man boss model / skeleton

---

## 8. UI / HUD

- [x] Kaleidoscope: medallion → C-button equip UI (`z_kaleido_scope` patch)
- [ ] Pause-screen L shortcut to beta inventory editor
- [ ] Magic bar visual tweaks for medallion-C usage
- [ ] Staff Roll presentation (text + chained flyovers)
- [ ] TwitchMessages sky-overlay rendering (optional)

---

## 9. Build / integration

- [ ] `soh/soh/Enhancements/TrifoolPercent/` module scaffold (CVar-gated)
- [ ] `ActorDB::AddEntry` registrations for every new actor
- [ ] `CustomMessageManager` table ID + registration
- [ ] `.o2r` build pipeline: scenes, rooms, objects, textures, Link anims, sequences
- [ ] Entrance-table init at `OnLoadGame` / `OnSceneInit`
- [x] New-file preset / `Statics_SetGameState` equivalent
- [ ] `SaveManager` custom section for TrifoolPercent flags
- [ ] Build the upstream `rom-setup/romhack` locally as a reference emulator target (not shipped) — see §0

---

## 10. Credits / licensing

- [ ] In-game Staff Roll lists all Triforce% contributors (LICENSE.txt condition 1)
- [ ] No-endorsement notice surfaced in About / README
- [ ] No Nintendo assets shipped — originals used for every "asset missing" item above

---

## 11. Explicitly out of scope (upstream delivery system)

For reference, do NOT port — SoH's resource system replaces all of this:

- `bootstrap/` (bootstrappers 1–4)
- `loader/fast_loader`, `loader/dma_patcher`, `loader/debugger`
- SRM/ACE setup, controller-data loader, `.pat` DMA diffs
- `toolchain/` build infrastructure (reference only)
