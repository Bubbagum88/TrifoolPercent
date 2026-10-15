# Triforce% source breakdown, and how each piece maps to Ship of Harkinian

Research notes for TrifoolPercent. Written 2026-10-15 from a read of the Triforce% source and the current Shipwright (SoH) `develop` tree.

- **Triforce% source:** https://github.com/triforce-percent/triforce-percent (last commit `12a4bb3`, 2023-08-06). Project site: https://gettriforce.link
- **SoH source checked against:** https://github.com/HarbourMasters/Shipwright at `6b4edce` (2026-10-14)
- **Credits for the run:** the in-game staff roll lists 25+ people. The director and lead programmer was Sauraen. Savestate was the runner, dwangoAC (TASBot) the executive producer, and MrCheeze and Savestate did the SRM/ACE setup.

---

## 1. TL;DR

1. The repo is really two things glued together:
   - **A delivery system:** SRM/ACE, bootstrapper chain, controller-data loader, and DMA patcher. It exists only to get custom code and assets into an unmodified N64 cartridge at runtime. **SoH needs none of it**, because SoH already loads mod assets and runs C/C++ natively.
   - **A normal OoT romhack** written against the decomp (z64hdr): about 20 new or rewritten actors, 4 new scenes, patches to about 15 vanilla scenes, a custom text table, 11 music sequences, voice samples, custom Link animations, and engine patches. **This is the part to port.**
2. `run/triforcepercent_data.run` is the master manifest of every injected file. It is the best checklist for TrifoolPercent's port (§3).
3. Triforce% added no new scene slots. It **hijacked existing ones**: Granny's Potion Shop became the Unicorn Fountain, the Goron Shop became the Triforce Room, Dampé's Hut became the Ending, and the vanilla Chamber of the Sages was replaced. It then rewired the entrance table to reach them. SoH can do the same thing with asset replacement. A cleaner approach needs code changes (§6).
4. **New actor code cannot ship as a pure `.otr/.o2r` asset mod.** SoH has a runtime `ActorDB` for registering new actors, but the actor's C code still has to be compiled into the SoH executable. Realistically, TrifoolPercent is **a SoH fork or branch (code) plus an `.o2r` asset pack (models, scenes, text, audio)**.
5. Several SoH enhancements already cover Triforce% mechanics: Bunny Hood speed/jump, Ocarina time travel without the Master Sword, custom music, and the save editor. Reuse them instead of porting the N64 patches.
6. **Missing assets:** the BotW Link/Zelda meshes and textures, the Running Man boss model, and all vanilla OoT data are **not in the repo**. The license requires crediting contributors in your in-game staff roll (§8).

---

## 2. What's in the repo

| Folder | What it is | Port? |
|---|---|---|
| `bootstrap/` | Bootstrappers 1–4: MIPS code delivered as controller inputs after SRM ACE | No (delivery only) |
| `loader/fast_loader` | "Hyperspeed loader". Polls controllers 8×/frame, CRC-checks 85-byte packets, writes them into Expansion Pak RAM, and also carries Twitch chat packets | No (Twitch idea optional) |
| `loader/dma_patcher` | Hooks `DmaMgr_ProcessMsg` and audio DMA to apply diff patches or serve whole files from RAM | No (SoH archive override does this) |
| `loader/debugger` | On-screen printf for dev | No |
| `statics/` | Engine patches and quest glue: game state, Lost Woods, shop, time travel, ocarina songs, interface, text loader, scene routing, cutscene terminators, audio, animation, hair physics, cel-shading microcode | **Yes, as SoH hooks** |
| `actor/` | ~20 custom actors and ~15 vanilla actor patches | **Yes** |
| `scene/` | 4 new scenes (Fast64 `.blend` plus exported C for 2) and diff patches for ~15 vanilla scenes | **Yes** |
| `model/` | Sages' Charm item model, Butterfly get-item model patch | Yes |
| `anim/` | 3 custom Link animations (LookToMeditate, Meditate, ReachUp) | Yes |
| `text/messages.c` | Full custom message table: quest dialogue, item gets, staff roll | **Yes** |
| `music/`, `voice/` | 11 `.aseq` sequences, 2 custom banks, 22 VADPCM voice/SFX samples | Yes |
| `textures/` | Item icons/names, Twitch emotes/badges | Partly |
| `toolchain/` | Makefiles, zzrtl ROM tools, TAS/m64 tools, submodules (z64hdr, novl, z64convert, ZAPD, z64audio, AudiobankToC, ExtLib) | Reference only |
| `rom-setup/` | Builds a "shortcut" ROM and a plain **romhack** ROM | Useful for testing (§7) |
| `run/` | Injection manifests (`triforcepercent_data.run`, `memes_data.run`) | **Checklist** |

---

## 3. How the run actually works (delivery system; don't port it)

Understanding this only explains why the code looks the way it does.

1. **SRM ACE.** Stale Reference Manipulation, a use-after-free, is performed live on an unmodified US 1.0 cart. It turns a stale actor pointer into a jump to controller 3's input data, which is executed as MIPS code.
2. **Bootstrapper 1** (`bootstrapper1.txt`). Controller 1 alternates `addi $gp, $zero, IIII` / `sh $gp, KKKK($s1)` button patterns, writing 2 bytes per frame into the text-font buffer at `0x801CA7A0`.
3. **Bootstrapper 2** (`bootstrapper2common.s`). Rewrites code in `padmgr` (`0x800A2630`) to call the controller buffer every poll, after flushing caches. It also restores the instruction SRM corrupted, so the game keeps running normally.
4. **Bootstrappers 3/4.** Same idea at 3× speed, then 2 words per poll via controllers 2–4, writing to an address counter at `0x80700000`.
5. **fast_loader.** Moves to 8 polls/frame with 85-byte CRC'd packets. Loads everything else into Expansion Pak RAM.
6. **`FIXED_AND_CALL_START`** loads and calls `debugger.bin`, `dma_patcher.bin`, and `statics.bin`.
7. **dma_patcher** intercepts every ROM file load and applies one of three techniques:
   - diff patches (`PATCH vrom file.pat`), a skip/write byte format;
   - whole-file replacement (`ACTOR`, `OBJECT`, `SCENE` swap the DMA table entry to RAM);
   - `STATICDATA` for music, banks, samples, animations, and icons that the audio and animation loaders read from RAM.

**Manifest command → SoH equivalent:**

| `.run` command | Meaning | SoH equivalent |
|---|---|---|
| `PATCH <vrom> x.pat` | Byte-diff a vanilla file (scene, room, actor overlay, texture) | Scenes/rooms/textures: replacement resource in `.o2r`. Actor overlays: C/C++ change or hook (overlays are native code in SoH) |
| `ACTOR <id> x.zovl` | Replace actor overlay slot `<id>` with new code | New C actor compiled into the fork, registered with `ActorDB` (or replace the vanilla actor's source) |
| `OBJECT <id> x.zobj` | Replace object file slot | New object resource (skeleton/DL/textures) in `.o2r` |
| `SCENE <id> x.zscene` | Replace scene slot | Scene/room resources in `.o2r` at that scene's path |
| `STATICDATA` | Music, bank, sample, anim, icon | Custom sequences/samples, Link anim resources, texture resources |

---

## 4. The quest, step by step, and the code behind it

The new plot is built from beta content and late-'90s urban legends. Line refs are into the Triforce% repo.

### 0. Setup
- `statics/statics.c` `Statics_SetGameState()` sets event flags (Epona, DMT not on fire, carpenters rescued, masks sold), gives magic and double magic, and makes adult Link's B button the Biggoron Sword so he never has the Master Sword.
- Press **L on the pause screen** to open the vanilla, unused **inventory editor** (`pauseCtx.debugState = 1`), shown as beta content.
- `gSaveContext.naviTimer = 1` keeps Navi quiet.

### 1. Key of Courage (Lost Woods → Bazaar → Skull Kid → Gerudo → Nabooru)
1. **Lost Woods secret path.** `Statics_LostWoods()` tracks exits North → West → South → West (`sWoodsPath {0,2,1,2}`) and then sets `BETAKOKIRI_SPAWNED`.
2. **Beta Kokiri** (`actor/BetaKokiri`, replaces actor 49, uses the unused beta Kokiri head/body objects). Asks for a flying bug (msgs `0x0B10`–`0x0B16`).
3. **Butterfly in the Bazaar.** `Statics_PatchShop()` swaps shelf slot 3 to a new Butterfly item after text `0x009C`. The Butterfly reuses the **Big Poe** item/GI/shop slots (`ITEM_BUTTERFLY = ITEM_BIG_POE`) with new icon, name and model patches (`model/object_gi_butterfly`, unused beta butterfly item model). `z_en_girla` is patched for the shop item.
4. Give the Butterfly and get **Magic Powder** (`Statics_MoveOddPotionToChild` moves it to the child trade slot).
5. **Skull Kid** (`actor/z_en_skj`, rewritten) recognises the powder and upgrades the **Bunny Hood** and **Gerudo Mask**:
   - The Bunny Hood gives run 1.5× and jump 1.25× (`Statics_Player_PatchedRunning`, `Statics_Player_BunnyHood_YSpeed`).
   - The working Gerudo Mask lets child Link pass the Gerudo.
6. **Gerudo guards** (`z_en_ge1` rewritten, `z_en_ge2` patched to cycle new dialogue `0x0B50`–`0x0B53`) let "her" into the Thieves' Hideout (`scene/gerudoway` patch).
7. **Nabooru** (`actor/Nabooru`, 536 lines) has a new shehnai model and three new animations, plus two in-engine cutscenes (`gerudoway_cs.c`) and a cutscene that pans to Desert Colossus (`scene/spot11` cutscene patch). She teaches the **"full Song of Time"** (internally *Long of Time*, defined in `statics/ocarina.c`). The ocarina also becomes chromatic (flat/sharp notes with D-pad modifiers) and uses the shehnai instrument for her teaching.
8. **Time travel anywhere.** Playing it calls `Statics_TimeTravel()`, which flips `linkAgeOnLoad` and reloads the current entrance, with no Master Sword needed. Supporting patches make the Master Sword pedestal (`z_bg_toki_swd`), Sheik (`z_en_xc`), Spiritual Stones (`z_demo_effect`) and the Player's pull-sword action (`z_player`) always treat Link as a child.

### 2. Key of Power (Running Man)
1. **Running Man race** (`actor/z_en_mm3`, rewritten): uses the Bunny Hood plus time travel so Link arrives first ("How did you get here before I did?!"). Timer hacks are `Statics_KeepTimerNegative` and the timer branch in `Statics_TimeTravel`.
2. **Running Man boss** (`actor/BossRunningMan`, 1,679 lines): a separate actor with a custom IK-rigged skeleton, AI by Rankaisija, and an intro cutscene. It spawns the exploding giant magenta rupee (`ACTOR_EN_EX_RUPPY`, beta).
3. Reward: the **Sages' Charm** (`model/SagesCharm`, reuses the Stone of Agony GI slot, new 24px icon). It enables:
   - **equipping Medallions to C buttons** (`statics/interface.c` `Statics_HandleEquipMedallionsToC`, `z_kaleido_scope` patch), a recreation of beta footage;
   - Fire Medallion on C performs Din's Fire (`z_player` patch);
   - magic bar patches;
   - a Zora's River → Zora's Domain warp.

### 3. Key of Wisdom (Zora's Domain → Unicorn Fountain)
1. **Zora's Domain ice melting** (`actor/z_bg_spot07_taki`, rewritten by z64me) plus scene and room patches for Zora's Domain.
2. The Domain → Lake Hylia exit is rerouted to the **Unicorn Fountain** (`statics/scene.c` `Statics_SetUpRouting`, entrance `0x0560`). It is a new scene (`scene/UnicornFountain`, Fast64 `.blend`) living in **Granny's Potion Shop's** scene slot, with Kakariko's door to Granny's turned into a turnaround.
3. Inside are the **Pedestal of the Ocarina** (`z_bg_mjin` copied as actor 23 with a different scale; vanilla warp pad) and the **Beta Great Fairy** (`actor/BetaGreatFairy`, a spiral rise/fall, sparkle particles, and the scene cutscene).
4. She teaches the **Overture of Sages**, a second custom song plus music (`OvertureOfSages.aseq`, `OvertureOfSagesWarp.aseq`).

### 4. Sacred Realm
1. Play the Overture in the **Temple of Time** "before the blade of destiny". `Statics_ShouldAbortWarp()` hijacks the warp-song path and spawns the **WSRCutscene** actor (`actor/WSRCutscene`, with a Temple of Time cutscene and a `tokinoma` room patch). Elsewhere it shows "You can't warp here!".
2. **Chamber of Sages** (`scene/ChamberOfSages`, replaces vanilla `kenjyanoma`):
   - custom scene draw config with animated water and light rays that fade near the player (`statics/scene.c` `Patched_CoSSceneDrawConfig`);
   - `CoSStaircases` (Rauru/Saria/Sheik staircases with collision, Triforce door, fade wall, hallway);
   - `CoSSages` drives the vanilla sage NPC actors through cutscene actions with fade-in particles. The dialogue is `0x0A10`–`0x0A77`, including the "Are you a boy or a girl?" Zelda/Sheik joke choice;
   - custom music `ChamberOfSages.aseq` with its own bank.
3. **Triforce Room** (`scene/TriforceRoom`, in the **Goron Shop** slot), entered via the Chamber exit with an entrance cutscene (`gEntranceCutsceneTable[30]`):
   - `TriforcePiece`: three pieces with a state machine (rise → combine → sink → disappear into Link), interpolated position, scale and brightness;
   - `TriforceChest`: beta Triforce chest model and animation;
   - `TriforceLight`;
   - custom Link animations (Meditate, ReachUp);
   - "You got the **Triforce**!!!" (`0x0ACE`, uses the beta Triforce icon);
   - the goddesses' wish with three choices (`0x0E00`–`0x0E07`);
   - music: `triforce.aseq`, `PowerOfTheGoddesses.aseq`.
4. **Ending** (`scene/Ending`, in **Dampé's Hut** slot, cutscene `FinaleCS`):
   - **BotW Link** and **BotW Zelda** with many baked animations, hair and tunic physics (`statics/hairphys.c`), and cel shading (`statics/f3dzex_celshading_patch.txt` plus Fast64 PR #122);
   - voice clips in English and Japanese (`voice/`), played by redirecting Ganon/boss SFX slots;
   - `SilentPrincess` flowers, `HolyLight` (`z_demo_holy`);
   - `TwitchMessages`, which renders live chat usernames and emotes in the sky;
   - music `HereTogether1-3` with a bank.
5. **Staff Roll** (`0x0F00`+): patched vanilla flyover cutscenes for Hyrule Field, DMT, Gerudo Valley, Deku Tree, Zora's Fountain, Zora's River, Kokiri Forest, Lon Lon, Wasteland and Castle (`scene/spot*/…Flyover.c`), chained by **cutscene terminators** in `statics/scene.c`. Music: `StaffRoll.aseq`, `Trailer.aseq`.

Unused or debug content worth knowing about:
- L+D-Down spawns the **Arwing**, which is the vanilla unused `En_Clear_Tag` actor (no port needed; it's already in SoH).
- `PatrickStar` and `MimicChest` are commented out.
- `Amogus` and `ocamungus.aseq` are the MAGFest 2023 "MEMES=1" build.

---

## 5. Piece-by-piece mapping to SoH

SoH facts below were checked in the current Shipwright tree:
- `soh/soh/ActorDB.h` (`ActorDB::AddEntry(name, desc, ActorInit)`);
- `soh/soh/Enhancements/game-interactor/GameInteractor_HookTable.h`;
- `.../vanilla-behavior/GIVanillaBehavior.h` (488 `VB_*` points);
- `soh/soh/Enhancements/custom-message/CustomMessageManager.h`;
- `soh/soh/ShipInit.hpp` (`RegisterShipInitFunc`);
- `Enhancements/BunnyHood.cpp`, `Enhancements/QoL/OcarinaTimeTravel.cpp`, `Enhancements/audio/`.

No `SceneDB` or `EntranceDB` exists in SoH today.

### Engine and quest glue (`statics/`) → SoH hooks in a TrifoolPercent module

| Triforce% | Mechanism there | SoH approach |
|---|---|---|
| `Statics_Player_Update` (called from patched `Player_UpdateCommon`) | Per-frame quest logic | `GameInteractor` hook `OnPlayerUpdate`, registered with `RegisterShipInitFunc` |
| `Statics_SetGameState` | Writes save flags | `OnLoadGame` hook or a new-file preset. Use `Flags_SetEventChkInf`/`Flags_SetInfTable` |
| Custom flags (`ootmain.h` lines 102–126) reuse unused `infTable`/`itemGetInf` bits | Reused vanilla bits | Use your own flags. SoH's save system can hold custom sections (`SaveManager`), which is cleaner than borrowing unused bits that rando may use |
| `Statics_LostWoods` | Position checks on scene exit | `OnSceneInit`/`OnTransitionEnd` plus `OnPlayerUpdate` |
| `Statics_PatchShop` | Edits shopkeeper overlay data at runtime | `OnActorInit` on `EnOssan`/`EnGirlA`, or the shop `VB_*` points. Randomizer's shop code is a good template |
| Butterfly / Sages' Charm reuse Big Poe / Stone of Agony slots | No room for new item IDs | SoH's `GetItemEntry` system (used by rando) supports custom items with custom draw functions. Start with slot reuse to get moving |
| Bunny Hood speed/jump | Patched Player code | **Already in SoH**: `BunnyHood.cpp` (`BUNNY_HOOD_FAST_AND_JUMP`). Gate it on your "powder applied" flag |
| Long of Time / `Statics_TimeTravel` | New song plus age flip | **Already in SoH**: `OcarinaTimeTravel.cpp` / `SwitchAge()`. For a new song, hook `OnOcarinaSongAction` and extend the song tables (`ocarina.c` has the note data to copy) |
| Chromatic ocarina (flats/sharps) | Code patches | SoH has `OnOcarinaNote` and ocarina-note `VB_*` points. Note bending may need code changes |
| Overture of Sages → `Statics_ShouldAbortWarp` | Hijacks the warp-song path | `OnWarpSongLeave` / `OnOcarinaSongAction` plus a scene check, then load the entrance |
| Medallions on C, Fire Medallion = Din's Fire | Kaleido and Player patches | `OnKaleidoscopeUpdate` plus a Player change. Probably needs a code edit in kaleido and `Player_ItemToItemAction` |
| `statics/message.c` + `text/messages.c` | Hooked text loader that checks a custom table first | **`CustomMessageManager::CreateMessage(tableID, textID, msg)`** plus the `OnOpenText` hook. The control codes (`BOX_BREAK`, `TWO_CHOICE`, `EVENT`, `FADE`, `QUICKTEXT`…) are standard OoT and port directly |
| Entrance rewiring (`Statics_SetUpRouting`) | Writes the entrance table at runtime | Edit `gEntranceTable` entries at init (the same trick works in SoH). For new scenes, see §6 |
| Cutscene terminators | Patches the vanilla terminator table | Cutscene `CS_TERMINATOR` handling is in C in SoH. Add cases, or detect the end of the cutscene in a hook and set `nextEntranceIndex` |
| `audio.c` (seq/bank/sample tables, SFX redirection) | Patched audio DMA | SoH loads sequences and samples as resources. Custom BGM is supported through its audio system (`AudioCollection`, `.ootrs` packs). New SFX/voice need new sample resources plus a code path to play them |
| `anime.c` (Link anims from RAM, SLERP morph) | Patched SkelAnime | Add Link animation resources to `.o2r` and reference them by path. SLERP is optional polish |
| `hairphys.c` | Plain C | Port as-is into the BotW actors |
| F3DZEX cel-shading microcode patch | RSP microcode change | **No microcode in SoH** (Fast3D renders GBI on the GPU). Needs a libultraship/shader-side change or a different technique. This is the hardest graphics item |
| `fast_loader` Twitch chat | Controller-data stream | Optional. SoH has a `Network/` layer (CrowdControl/Sail-style) that could feed a chat actor |

### Actors

| Actor | Kind | SoH approach |
|---|---|---|
| BetaKokiri, BossRunningMan, Nabooru (new), BetaGreatFairy, CoSSages, CoSStaircases, TriforcePiece, TriforceChest, TriforceLight, WSRCutscene, BotWLink, BotWZelda, HolyLight, SilentPrincess, TwitchMessages | New actors in C against decomp/z64hdr | Port the C nearly 1:1 (SoH is the same decomp code, though some function names differ: e.g. `GlobalContext` → `PlayState`, `func_8010B680` → `Message_StartTextbox`). Register each with `ActorDB::AddEntry` and put their objects in `.o2r` |
| z_en_skj, z_en_ge1, z_en_mm3, z_bg_spot07_taki, z_en_owl (killed) | Full replacements of vanilla actors | Either edit those actors' sources in the fork, or keep vanilla and spawn your own variants. `z_en_owl` is simply "always kill" (`ShouldActorInit` hook) |
| z_player, z_kaleido_scope, z_en_xc, z_demo_effect, z_bg_toki_swd, z_en_ge2, z_en_girla, z_oceff_wipe, z_bg_mjin | MIPS instruction patches (`actorpatch.txt`) | Each comment block describes intent ("always treat Link as child", "Din's Fire on C"…). Re-implement as `VB_*` hooks or small source edits. Don't try to translate the assembly |

### Scenes

| Scene | Source in repo | SoH approach |
|---|---|---|
| Unicorn Fountain, Chamber of Sages, Triforce Room, Ending | Fast64 `.blend` (+ exported C for Triforce Room and Ending) | Re-export with Fast64 (OoT decomp C), then build resources into `.o2r`. Title cards: `title.ia8.png` |
| ~15 vanilla scene/room patches (Lost Woods room 5/7, Bazaar, Gerudo hideout, Desert Colossus, Zora's Domain, Temple of Time, staff-roll flyovers) | Byte diffs against original files (md5-pinned) | Turn each diff into readable intent: actor list additions, cutscene data (`*Flyover.c` already in C), etc. Apply as resource replacements or `OnSceneSpawnActors` actor spawns |

### Assets

| Asset | In repo? | Note |
|---|---|---|
| Triforce piece, Triforce chest, Triforce light | Yes (`.blend`, `.objex`, `.zobj`) | Good first import test |
| Unicorn Fountain, Chamber of Sages, Triforce Room, Ending scenes | Yes (`.blend`) | The Ending `.blend` is ~4 MB |
| Sages' Charm model, CoS staircases, Nabooru shehnai | Yes | |
| BotW Link/Zelda meshes and textures | **No** (only animations and `*Mesh.h` headers; mesh `.c` and eye `.inc` are missing) | Derivative of Nintendo assets per Sauraen. You'd need your own models |
| Running Man boss model | **No** (header only) | Same |
| Music `.aseq`, banks, voice `.vadpcm` | Check per file | Sequences need conversion to SoH's sequence format |
| Vanilla OoT data (beta Kokiri objects, beta fairy, butterfly model, etc.) | No (comes from the ROM) | SoH extracts these from the user's ROM, so it's fine |

---

## 6. Recommended architecture for TrifoolPercent

1. **Code (fork or branch of Shipwright).** Add a `soh/soh/Enhancements/TrifoolPercent/` module behind a CVar:
   - quest-logic hooks (§5);
   - new actors registered via `ActorDB`;
   - custom messages via `CustomMessageManager`;
   - entrance rewiring.
   This is the only way to ship new actor behaviour.
2. **Assets (`.o2r` in `mods/`).** Scenes, rooms, objects, textures, Link animations, and sequences.
3. **Scene slots.** Start the way Triforce% did: reuse rarely-visited scene slots and rewire entrances (zero engine work). Graduate to adding real scene-table entries later. There's no `SceneDB` in SoH yet, so that's a scene-table change in the fork.
4. **Order of work.** Each step proves one pipeline end-to-end:
   1. Triforce Room scene + TriforcePiece/Chest/Light actors + the `0x0ACE` text. This proves scene, actor, object and message.
   2. Chamber of Sages + Overture of Sages song. This proves song, warp and cutscene.
   3. Unicorn Fountain + Beta Great Fairy.
   4. The Courage/Power/Wisdom side quests, reusing SoH's Bunny Hood and time-travel features.
   5. Ending, staff roll, and cel shading last.

## 7. Practical tips

- **Build their romhack first** (`rom-setup/romhack`, needs Linux/WSL, the glankk N64 toolchain, and your OoT 1.0U ROM). Running it in an emulator gives you a reference for every scene and cutscene while porting. Emulators can't run the live ACE version, but the romhack works.
- Use `run/triforcepercent_data.run` as your porting checklist, one line per asset.
- `statics/statics.c` `Statics_TestShortcuts` has commented-out warps to every story beat. These are good test entry points to mirror with SoH's debug warp.

## 8. Licensing and credit

`LICENSE.txt` (Triforce% Team, 2019–2022) allows use and modification on three conditions:
1. **Credit the contributors.** If you use content directly in a romhack-like project, their names must appear **in your in-game Staff Roll**.
2. **No implied endorsement.**
3. **No warranty.**

The repo contains no Nintendo assets. The BotW models and the Running Man boss skeleton were deliberately left out.

## 9. Corrections for `docs/references/` in the TrifoolPercent repo

I checked each GitHub link in `assets.md` and `creators.md` with `git ls-remote`.

**These repos resolve:**
- `HarbourMasters/Shipwright`
- `Fast-64/fast64`

**These don't exist (or aren't public):**
- `TriforcePercent/TriforcePercent` → the real repo is **`triforce-percent/triforce-percent`**. `docs/references/triforcepercent/index.md` already has it right.
- `HarbourMasters/unicorn-fountain`
- `HarbourMasters/oot-beta-restoration`
- `HarbourMasters/soh-modloader`
- `HarbourMasters/soh-tools`
- `OoT-Randomizer/oot-ace`
- `matthewkirby/srm-oot`
- `zeldamods/ZAPD` → the real one is `zeldaret/ZAPD`
- `zeldamods/blender-oot-tools`
- `Emill/N64Recomp` → the real one is `N64Recomp/N64Recomp`
- `OoT-Randomizer/OoT-Randomizer` didn't resolve from here either. OoTR's main repo is commonly `TestRunnerSRL/OoT-Randomizer` (not checked).

**The roadmap's "Arwing actor"** is vanilla `En_Clear_Tag` and already exists in SoH; Triforce% just spawned it. It needs a spawn trigger, not a port.

The useful real tools are the Triforce% submodules: `z64me/novl`, `turpaan64/z64hdr`, `turpaan64/z64convert`, `zeldaret/ZAPD`, `z64tools/z64audio`, `sauraen/AudiobankToC`, `sauraen/ci4tool`, and `rankaisija64/ExtLib`.

## Sources
- Triforce% repo: https://github.com/triforce-percent/triforce-percent
- TASVideos forum abstract, with the credits and the technical write-up by the team: https://tasvideos.org/Forum/Posts/516859
- Shipwright: https://github.com/HarbourMasters/Shipwright
