# TrifoolPercent Roadmap

This is the plan for getting from an empty repo to the full Triforce% story playing natively in Ship of Harkinian (SoH). It builds on the research in [code-breakdown.md](references/triforcepercent/code-breakdown.md) (section numbers below point into it) and the tools listed in the [README](../README.md#tools).

## What "success" means

A player with a legal OoT ROM can install TrifoolPercent, start a new file, and play the Triforce% quest from the Lost Woods to the staff roll in SoH. No ACE and no memory corruption. That means:

- every story beat from the run is reachable in order, with no debug warps;
- the Triforce% team is credited in the in-game staff roll, as their license requires (§8);
- it installs from a release with clear steps, and vanilla SoH saves keep working.

## How the mod is built

The research settled the shape of the project (§1, §6):

| Part | What goes in it | Why |
|---|---|---|
| **Code**: a Shipwright fork with a `soh/soh/Enhancements/TrifoolPercent/` module behind one CVar | New actors (registered with `ActorDB`), quest logic on `GameInteractor` hooks, custom text via `CustomMessageManager`, entrance rewiring | New actor behaviour has to be compiled into SoH. An `.o2r` alone can't ship it |
| **Assets**: an `.o2r` archive in SoH's `mods/` folder | Scenes, rooms, objects, textures, Link animations, music | SoH loads these as resource overrides |
| **This repo** | Source assets (`.blend`), actor and quest source, docs, build scripts | One place to track the port. Code is synced into the fork |

**Scene slots.** Start the way Triforce% did: reuse rarely visited vanilla scene slots (Goron Shop → Triforce Room, Granny's Potion Shop → Unicorn Fountain, Dampé's Hut → Ending) and rewire entrances. Adding real scene-table entries is a later, optional cleanup (Phase 6).

**Reuse before porting.** SoH already has Bunny Hood speed/jump, Ocarina time travel without the Master Sword, custom music, and the save editor. The Arwing is vanilla `En_Clear_Tag` and only needs a spawn trigger.

## Phases

Each phase proves one new pipeline end to end, so later phases are mostly repetition of a solved problem. A phase is done when its exit check passes, not when its checklist is ticked.

### Phase 0: Foundation ✅

- [x] Repo and folder structure
- [x] Triforce% source breakdown and SoH mapping
- [x] Tools list in the README, dead reference links fixed

### Phase 1: Proof of concept, the Triforce Room (in progress)

Goal: prove that a custom scene, custom actors, a custom object and custom text all work in SoH. This is the riskiest part of the plan, and everything after it depends on it.

- [ ] Fork Shipwright and get a clean local build with an extracted `oot.o2r`
- [ ] Add the `TrifoolPercent` module and CVar, with one `RegisterShipInitFunc` entry point
- [ ] Re-export `scene/TriforceRoom` with Fast64 and pack it into the Goron Shop slot in an `.o2r`
- [ ] Port `TriforcePiece`, `TriforceChest` and `TriforceLight` and register them with `ActorDB`
- [ ] Import the Triforce piece, chest and light objects
- [ ] Show "You got the Triforce!!!" (`0x0ACE`) through `CustomMessageManager`
- [ ] Document the build steps in `docs/build.md` so anyone can reproduce them

**Exit check:** warp into the Goron Shop entrance in SoH, watch the three pieces rise, combine and sink into Link, and get the Triforce text box.

### Phase 2: Toolchain and workflow

Goal: make Phase 1 repeatable before scaling it up.

- [ ] One script that builds the `.o2r` from this repo's sources
- [ ] Decide how this repo and the fork stay in sync (git submodule, or code kept only in the fork)
- [ ] A dev save or debug warp for each story beat, mirroring `Statics_TestShortcuts`
- [ ] Build the Triforce% romhack ROM (`rom-setup/romhack`) as a side-by-side reference (§7)
- [ ] CI that at least builds the fork
- [ ] Turn `run/triforcepercent_data.run` into a tracked porting checklist in `docs/references/import_status.md`

**Exit check:** a fresh clone can produce a playable Triforce Room build by following `docs/build.md`.

### Phase 3: The Sacred Realm (Chamber of Sages and Overture of Sages)

Goal: prove custom songs, warps and in-engine cutscenes.

- [ ] Overture of Sages as a new ocarina song (`OnOcarinaSongAction`, note data from `ocarina.c`)
- [ ] Playing it in the Temple of Time triggers the `WSRCutscene` warp; elsewhere it says "You can't warp here!"
- [ ] Chamber of Sages scene with its water and light-ray draw config
- [ ] `CoSStaircases` and `CoSSages`, including the sage dialogue (`0x0A10`–`0x0A77`)
- [ ] Chamber exit leads into the Triforce Room with its entrance cutscene
- [ ] First custom music: `ChamberOfSages.aseq`, `triforce.aseq`, `PowerOfTheGoddesses.aseq`
- [ ] The goddesses' wish with its three choices (`0x0E00`–`0x0E07`)

**Exit check:** play the Overture in the Temple of Time and reach the Triforce from there with no debug help.

### Phase 4: Key of Wisdom (Zora's Domain and the Unicorn Fountain)

- [ ] Zora's Domain ice melting (`z_bg_spot07_taki`)
- [ ] Reroute the Domain → Lake Hylia exit to the Unicorn Fountain (Granny's Potion Shop slot)
- [ ] Unicorn Fountain scene, Pedestal of the Ocarina, and the Beta Great Fairy with her cutscene
- [ ] She teaches the Overture of Sages (replacing the Phase 3 debug unlock)

**Exit check:** from a fresh file with the right flags, learn the Overture from the Beta Great Fairy.

### Phase 5: Keys of Courage and Power (the side quests)

Key of Courage:
- [ ] New-file game state and Navi silence (`Statics_SetGameState`)
- [ ] Lost Woods secret path spawns the Beta Kokiri
- [ ] Butterfly in the Bazaar (start with the Big Poe slot, as Triforce% did), traded for Magic Powder
- [ ] Rewritten Skull Kid upgrades the Bunny Hood (SoH's existing feature, gated on a flag) and Gerudo Mask
- [ ] Gerudo guards and the Thieves' Hideout path
- [ ] Nabooru with her shehnai, cutscenes, and the "full Song of Time"
- [ ] Time travel anywhere (SoH's `OcarinaTimeTravel`) and the "always a child" patches

Key of Power:
- [ ] Running Man race rewrite (`z_en_mm3`)
- [ ] Running Man boss. **Needs a new model**: the original isn't in the Triforce% repo
- [ ] Sages' Charm: Medallions on C, Fire Medallion as Din's Fire, the Zora's River warp

**Exit check:** all three keys can be earned in order on one save file.

### Phase 6: Ending, staff roll and polish

- [ ] Ending scene in Dampé's Hut slot with `FinaleCS`, Silent Princess flowers and Holy Light
- [ ] **BotW Link and Zelda models: needs new models**, since the originals were left out of the repo
- [ ] Hair and tunic physics (`hairphys.c` ports as plain C)
- [ ] Cel shading. The hardest graphics item: SoH has no microcode, so it needs a shader-side approach in libultraship, or a cheaper look
- [ ] Voice clips and remaining music (`HereTogether1-3`, `StaffRoll.aseq`, `Trailer.aseq`)
- [ ] Staff roll flyovers chained by cutscene terminators, **with the Triforce% team credited**
- [ ] Optional: real scene-table entries instead of hijacked slots
- [ ] Optional: live chat in the sky (`TwitchMessages`) through SoH's network layer

**Exit check:** a full playthrough from new file to the end of the staff roll.

### Phase 7: Release

- [ ] Playtest the full quest, fix soft-locks and missing flags
- [ ] Confirm vanilla saves and other SoH features still work with the CVar off
- [ ] Credits and licensing pass on every asset (`docs/references/creators.md`)
- [ ] Release as a drag-and-drop package (see below), with install steps
- [ ] List it on modding sites such as GameBanana, with downloads for the base mod and the optional HD textures
- [ ] Community testing round

**Goal: drag-and-drop install.** The finished mod should be one package a player drops into SoH's `mods` folder (or installs through a common mod loader), with no custom SoH build.

That conflicts with how the mod is built today. Assets can already ship as a plain `.o2r`, but new actor code has to be compiled into SoH, which is why the plan uses a Shipwright fork. Three ways to close that gap, best first:

1. **SoH code mods.** libultraship has a newer C mod system (`manifest.json` plus events, see its scripting guide). If the SoH release we target can load compiled code from a mod package, the actors and quest logic ship inside the package and the fork goes away. Check this early, in Phase 2, because it changes how all the code is organised.
2. **Upstream the module.** Get the `TrifoolPercent` module (or the generic hooks it needs) merged into Shipwright behind an off-by-default toggle. The download is then just the `.o2r`.
3. **Fallback.** Ship a prebuilt SoH build together with the `.o2r` in one zip. Still one download, but not drag-and-drop into an existing install.

Either way, keep all code inside the module and hooks from Phase 1, so it can move to option 1 or 2 without a rewrite.

## Textures: original and HD

Every texture we make ships in two versions, so players can pick the look that matches the rest of their game:

- **Original:** N64 resolution and formats, matching vanilla OoT. This is the default and must always work on its own.
- **HD:** upscaled versions made to sit alongside HD texture packs such as OoT Reloaded, so our scenes don't look low-res next to an HD Hyrule.

How we plan to do it:

- **One source, two exports.** Keep each texture's master in `assets/textures/` at the higher resolution, and generate the N64 version from it with a script, so the two sets never drift apart.
- **Same resource paths.** The HD texture replaces the original at the same path, so the code and scenes don't care which one is loaded. SoH can load higher-resolution replacements and has an alternate-assets toggle; confirm the exact mechanism in the Phase 2 toolchain work.
- **Packaging.** Ship the HD set as a separate, optional `.o2r` next to the base archive, so the base mod stays small and the HD pack can be updated on its own.
- **Load order with other HD packs.** Our hijacked scene slots (Goron Shop, Granny's Potion Shop, Dampé's Hut) are also retextured by HD packs. Test that our textures win in those scenes, and document the load order players need.
- **Free wins.** Where our scenes reuse vanilla textures, leave them as vanilla references, so an installed HD pack upgrades them for free.

Checklist:
- [ ] Phase 2: decide the HD mechanism (separate archive or alternate assets) and add the HD export to the build script
- [ ] Phase 1 onward: every new texture lands with both an original and an HD version
- [ ] Phase 7: test with no HD pack, with our HD pack only, and alongside OoT Reloaded

## Biggest risks

| Risk | Where it bites | Plan |
|---|---|---|
| Fast64 scene export doesn't load cleanly in SoH | Phase 1 | That's why the Triforce Room goes first. Fall back to Torch/ZAPDTR resource definitions if needed |
| Keeping a Shipwright fork current | Every phase | Keep all changes inside the `TrifoolPercent` module and hooks, so rebasing stays small |
| Missing models (BotW Link/Zelda, Running Man boss) | Phases 5–6 | Commission or build new ones, or swap in stand-ins. Not blocking until late |
| Cel shading has no direct SoH equivalent | Phase 6 | Treat as stretch. Ship without it if needed |
| Custom flags clashing with randomizer | Phases 3–5 | Use a custom save section in `SaveManager`, not unused vanilla bits |
| Drag-and-drop needs SoH to load our code without a fork | Phases 2, 7 | Check SoH code mods in Phase 2; otherwise upstream the module or ship a bundled build |
| Our textures clash with, or get overridden by, HD texture packs | Phases 1–7 | Keep both texture sets, test with OoT Reloaded installed, and document the load order |

## Stretch goals

Multiple endings, randomizer compatibility, the chromatic ocarina, extra beta content (Arwing spawn, unused rooms), and custom music beyond the run's soundtrack.
