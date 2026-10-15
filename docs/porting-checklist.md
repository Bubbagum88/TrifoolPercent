# TrifoolPercent Porting Checklist

Every mechanic, scene, actor, and asset from the original [Triforce%](https://github.com/triforce-percent/triforce-percent) run that needs to land in TrifoolPercent. Grouped by category, with each item an unchecked box.

Sourced from [docs/references/triforcepercent/code-breakdown.md](references/triforcepercent/code-breakdown.md) and the upstream `run/triforcepercent_data.run` manifest. Items marked **(already in SoH)** can be gated on a TrifoolPercent flag rather than reimplemented. Items marked **(asset missing)** are not in the upstream repo (BotW/derivative) and need an original replacement.

---

## 1. Scenes

New scenes (Fast64 `.blend` sources upstream, re-export to `.o2r`):

- [ ] Unicorn Fountain (lives in Granny's Potion Shop scene slot)
- [ ] Chamber of Sages (replaces vanilla `kenjyanoma`)
- [ ] Triforce Room (lives in Goron Shop scene slot) — POC underway in Besitu slot
- [ ] Ending (lives in Dampé's Hut scene slot)

Vanilla scene / room patches (reapply as resource replacements or `OnSceneSpawnActors`):

- [ ] Lost Woods rooms 5 and 7 (secret-path trigger wiring)
- [ ] Hyrule Market Bazaar (shop shelf swap for Butterfly)
- [ ] Gerudo Hideout / `gerudoway` (child-Link entry, Nabooru cutscenes)
- [ ] Desert Colossus / `spot11` (Nabooru pan cutscene)
- [ ] Zora's Domain (ice-melt, exit reroute to Unicorn Fountain)
- [ ] Kakariko (Granny's door turnaround)
- [ ] Temple of Time / `tokinoma` room patch (WSR warp spawn)
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
- [ ] Entrance-table rewiring (`Statics_SetUpRouting`): Zora's Domain → Unicorn Fountain (`0x0560`), Chamber → Triforce Room, Triforce Room → Ending, plus the staff-roll chain
- [ ] Entrance cutscene table entry 30 (Triforce Room entry)
- [ ] Cutscene terminator chain in `statics/scene.c` (sequence staff-roll flyovers)

---

## 2. Custom actors (new code — needs ActorDB registration + object in `.o2r`)

- [ ] `BetaKokiri` (replaces actor slot 49; reuses beta Kokiri head/body)
- [ ] `Nabooru` (new 536-line actor, shehnai model, 3 anims, 2 in-engine cutscenes)
- [ ] `BossRunningMan` (1,679 lines, custom IK skeleton, AI, intro cutscene) — **(asset missing: boss skeleton)**
- [ ] `BetaGreatFairy` (rise/fall, sparkle particles, scene cutscene)
- [ ] `WSRCutscene` (Warp-Song-Replacement Temple of Time cutscene actor)
- [ ] `CoSStaircases` (Rauru/Saria/Sheik staircases, collision, Triforce door, fade wall, hallway)
- [ ] `CoSSages` (drives vanilla sage NPCs, fade-in particles, dialogue 0x0A10–0x0A77)
- [ ] `TriforcePiece` (3 pieces, state machine: rise → combine → sink → absorb)
- [ ] `TriforceChest` (beta chest model + animation)
- [ ] `TriforceLight`
- [ ] `BotWLink` (baked animations, hair/tunic physics, cel shading) — **(asset missing: mesh/textures)**
- [ ] `BotWZelda` (baked animations, hair/tunic physics, cel shading) — **(asset missing: mesh/textures)**
- [ ] `HolyLight` / `z_demo_holy`
- [ ] `SilentPrincess` (flowers)
- [ ] `TwitchMessages` (live chat usernames/emotes rendered in sky)

Rewritten vanilla actors (full replacement — fork the source or spawn custom variant):

- [ ] `z_en_skj` (Skull Kid: recognises Magic Powder, upgrades Bunny Hood / Gerudo Mask)
- [ ] `z_en_ge1` (Gerudo guard, lets child Link through)
- [ ] `z_en_mm3` (Running Man race)
- [ ] `z_bg_spot07_taki` (Zora's Domain ice-melt, by z64me)
- [ ] `z_en_owl` (always-kill: skip Kaepora dialogue)

Patched vanilla actors (upstream ships as MIPS diffs — reimplement as `VB_*` hooks / small edits):

- [ ] `z_player` (always-child treatment for sword/pedestal, Din's Fire on C, Bunny Hood run/jump)
- [ ] `z_kaleido_scope` (Medallions to C buttons, magic-bar patches)
- [ ] `z_en_xc` (Sheik treats Link as child)
- [ ] `z_demo_effect` (Spiritual Stones treat Link as child)
- [ ] `z_bg_toki_swd` (Master Sword pedestal: no-op pull-sword)
- [ ] `z_en_ge2` (Gerudo guard dialogue cycle 0x0B50–0x0B53)
- [ ] `z_en_girla` (shop item: Butterfly in Bazaar slot 3)
- [ ] `z_oceff_wipe` (ocarina / warp FX)
- [ ] `z_bg_mjin` (Pedestal of the Ocarina: scale variant, actor slot 23)

---

## 3. Mechanics / engine glue (`statics/`)

Quest state and flow:

- [ ] `Statics_SetGameState` — set event flags (Epona, DMT not on fire, carpenters, masks sold), grant magic + double magic, give adult B = Biggoron Sword, `naviTimer = 1`
- [ ] Beta inventory editor on pause (`L` → `pauseCtx.debugState = 1`)
- [ ] `Statics_LostWoods` — N→W→S→W path tracking, set `BETAKOKIRI_SPAWNED`
- [ ] `Statics_PatchShop` — Bazaar shelf-3 swap to Butterfly after text `0x009C`
- [ ] `Statics_MoveOddPotionToChild` — Butterfly reward → child trade slot
- [ ] Custom save flags (prefer SoH `SaveManager` custom section over borrowing `infTable` / `itemGetInf` bits)
- [ ] `Statics_SetUpRouting` — runtime entrance-table rewiring
- [ ] `Statics_TestShortcuts` — debug warps to every story beat (mirror with SoH debug warp)
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
- [ ] Zora's River → Zora's Domain warp (Sages' Charm feature)

Ocarina / songs:

- [ ] Chromatic ocarina (flat/sharp with D-pad modifiers)
- [ ] Shehnai instrument timbre during Nabooru teaching
- [ ] "Long of Time" / full Song of Time definition (`statics/ocarina.c`)
- [ ] `Statics_TimeTravel` — flip `linkAgeOnLoad`, reload entrance, no Master Sword required **(already in SoH: `OcarinaTimeTravel.cpp`)**
- [ ] Overture of Sages (new song, note data + table extension)
- [ ] `Statics_ShouldAbortWarp` — Overture hijacks warp-song path in Temple of Time
- [ ] "You can't warp here!" message elsewhere

Running Man race:

- [ ] Timer branch in `Statics_TimeTravel`
- [ ] `Statics_KeepTimerNegative`
- [ ] Spawn `ACTOR_EN_EX_RUPPY` (exploding giant magenta rupee)

Rendering / animation:

- [ ] Custom Link animations: LookToMeditate
- [ ] Custom Link animations: Meditate
- [ ] Custom Link animations: ReachUp
- [ ] SLERP morph on patched SkelAnime (optional polish)
- [ ] `hairphys.c` — hair / tunic physics for BotW actors
- [ ] F3DZEX cel-shading microcode (BotW Link/Zelda) — needs libultraship / shader-side approach, no RSP microcode in SoH
- [ ] Chamber of Sages scene draw config: animated water + light rays fading near player
- [ ] Spawn trigger for Arwing (vanilla `En_Clear_Tag`, L+D-Down) **(already in SoH)**

Audio:

- [ ] SFX redirection table (Ganon/boss SFX slots repurposed for voice)
- [ ] Voice playback path (new sample resources + play code path)

---

## 4. Cutscenes

- [ ] Nabooru intro / teaching cutscenes (`gerudoway_cs.c`)
- [ ] Desert Colossus pan cutscene (`spot11`)
- [ ] Running Man boss intro cutscene
- [ ] Beta Great Fairy scene cutscene (Unicorn Fountain)
- [ ] WSR Temple of Time cutscene (Overture played at pedestal)
- [ ] Chamber of Sages cutscene (sage fade-in, dialogue, Zelda/Sheik reveal)
- [ ] Triforce Room entrance cutscene (`gEntranceCutsceneTable[30]`)
- [ ] Triforce Room sequence (pieces rise → combine → sink → absorb)
- [ ] Goddesses' wish three-choice sequence (`0x0E00`–`0x0E07`)
- [ ] Ending `FinaleCS` (BotW Link + Zelda, Silent Princess, HolyLight)
- [ ] Staff-roll flyover chain terminators (`statics/scene.c`)

---

## 5. Text / messages (`text/messages.c`)

Port via `CustomMessageManager::CreateMessage(tableID, textID, msg)`. Standard OoT control codes (`BOX_BREAK`, `TWO_CHOICE`, `EVENT`, `FADE`, `QUICKTEXT`) port directly.

- [ ] Beta Kokiri dialogue (`0x0B10`–`0x0B16`)
- [ ] Bazaar Butterfly text (`0x009C`)
- [ ] Gerudo guard rotation (`0x0B50`–`0x0B53`)
- [ ] Chamber of Sages dialogue (`0x0A10`–`0x0A77`, incl. boy/girl choice)
- [ ] "You got the Triforce!!!" (`0x0ACE`, uses beta Triforce icon)
- [ ] Goddesses' wish three-choice (`0x0E00`–`0x0E07`)
- [ ] Staff Roll text (`0x0F00`+)
- [ ] "You can't warp here!" fallback for non-ToT Overture plays
- [ ] Butterfly / Sages' Charm / Overture item-get text
- [ ] Nabooru / Skull Kid / Running Man quest dialogue
- [ ] Beta Great Fairy teaching dialogue

---

## 6. Audio (sequences + banks + samples)

Music sequences (11 `.aseq` upstream; convert to SoH sequence format):

- [ ] `OvertureOfSages.aseq`
- [ ] `OvertureOfSagesWarp.aseq`
- [ ] `ChamberOfSages.aseq` (+ custom bank)
- [ ] `triforce.aseq`
- [ ] `PowerOfTheGoddesses.aseq`
- [ ] `HereTogether1.aseq` (+ bank)
- [ ] `HereTogether2.aseq`
- [ ] `HereTogether3.aseq`
- [ ] `StaffRoll.aseq`
- [ ] `Trailer.aseq`
- [ ] 11th sequence (verify slot vs. upstream manifest)

Instrument banks:

- [ ] Shehnai bank (Nabooru ocarina)
- [ ] Chamber of Sages bank
- [ ] HereTogether bank

Voice / SFX samples (22 VADPCM):

- [ ] BotW Link voice clips (English)
- [ ] BotW Link voice clips (Japanese)
- [ ] BotW Zelda voice clips (English)
- [ ] BotW Zelda voice clips (Japanese)
- [ ] Remaining ambience / SFX samples (crosscheck upstream `voice/`)

---

## 7. Models / objects / textures

Objects (upstream has these):

- [ ] Triforce piece object (`.zobj`)
- [ ] Triforce chest object (beta model + animation)
- [ ] Triforce light object
- [ ] Sages' Charm GI model
- [ ] Butterfly GI model (`object_gi_butterfly`, beta item mesh)
- [ ] Nabooru shehnai object
- [ ] CoS staircases geometry
- [ ] Unicorn Fountain scene geometry (Fast64 `.blend`)
- [ ] Chamber of Sages scene geometry
- [ ] Triforce Room scene geometry
- [ ] Ending scene geometry (~4 MB `.blend`)

Textures / UI:

- [ ] Butterfly item icon + name
- [ ] Sages' Charm icon (24px)
- [ ] Beta Triforce item icon (for `0x0ACE`)
- [ ] Title cards (`title.ia8.png`) for each new scene
- [ ] Twitch emotes
- [ ] Twitch badges

Assets NOT in upstream — originals required:

- [ ] BotW Link mesh + textures + eye `.inc`
- [ ] BotW Zelda mesh + textures + eye `.inc`
- [ ] Running Man boss model / skeleton

---

## 8. UI / HUD

- [ ] Kaleidoscope: medallion → C-button equip UI (`z_kaleido_scope` patch)
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
- [ ] New-file preset / `Statics_SetGameState` equivalent
- [ ] `SaveManager` custom section for TrifoolPercent flags
- [ ] Build the upstream `rom-setup/romhack` locally as a reference emulator target (not shipped)

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
