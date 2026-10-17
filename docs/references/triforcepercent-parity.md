# TrifoolPercent vs Triforce% — patch comparison

Written 2026-10-17. Scope: logic / engine patches only. Custom scenes, custom actors, and cutscene content are intentionally rebuilt and are **out of scope**; this report only looks at where the two projects produce the same *mechanical effect* and whether our implementation is sound, light, or missing.

Source material:
- Upstream breakdown: `research/triforce-percent-code-breakdown.md`
- Our module: `soh/Enhancements/TrifoolPercent/` (3,514 LOC across 16 glue files, PRs #45–#54 merged)

---

## 1. Comparison table

| # | Mechanic | Triforce% (how) | TrifoolPercent (how) | Verdict |
|---|---|---|---|---|
| 1 | Starting save state (Epona, DMT fire out, carpenters, masks, Saria, magic, adult B = Biggoron, Navi quiet) | `Statics_SetGameState` — writes flags once | `Setup.cpp:36` `OnLoadGame` one-shot, same flag set, `bgsFlag=1`, `naviTimer=1`, `doubleMagic` | **Equivalent** |
| 2 | Pause-L inventory editor | `pauseCtx.debugState = 1` on L | `Setup.cpp:102`, same | **Equivalent** |
| 3 | Dev warps / Arwing | `Statics_TestShortcuts` warp list; L+DDown spawns `EN_CLEAR_TAG` | `Setup.cpp:124,157` — L+DDown Arwing, L+DRight cycles 0x0560 / 0x006B / 0x00CD | **Equivalent (ours a touch leaner)** |
| 4 | Door of Time open for adult | Not an explicit upstream patch — upstream never has adult Link without Master Sword touching that door; this is a *new* requirement our timeline creates | `TrifoolPercent.cpp:298` — on ToT scene spawn as adult, set `EVENTCHKINF_OPENED_DOOR_OF_TIME` and `drawParams[1]=0xFF` | **TrifoolPercent-specific (correct for us)** |
| 5 | Chromatic song recognizer | `statics/ocarina.c` — patches note-collapse to allow flats/sharps, adds Long/Overture tables | `SongEngine.cpp` — parallel listener on `OnOcarinaNote`, 12-note sliding window, `(note,bend)→semitone` encoding, song table | **Similar effect, different approach.** Ours runs alongside vanilla instead of mutating `sNoteValueIndexMap`. Safer, but see §2 (Long of Time not consumed) |
| 6 | "Long of Time" / full Song of Time | Teaches new song, `Statics_TimeTravel` flips `linkAgeOnLoad` on recognition, reloads entrance | **Armed in table only** (`SongEngine.cpp` entry). No consumer. No age-flip, no entrance reload | **Missing — consumer side** |
| 7 | Overture of Sages | Teaches new song, `Statics_ShouldAbortWarp` hijacks warp-song path, spawns WSRCutscene | Scaffolded, `isArmed=false`. Not wired to Beat 3 | **Missing — scaffolded only** |
| 8 | Always-treat-Link-as-child patches (`z_bg_toki_swd`, `z_en_xc` Sheik, `z_demo_effect` stones, player pull-sword) | MIPS instruction patches | Not implemented | **Missing.** Needed once time-travel-anywhere is wired, otherwise adult Link in ToT re-pulls Master Sword |
| 9 | Bunny Hood run 1.5× / jump 1.25× | `Statics_Player_PatchedRunning`, `Statics_Player_BunnyHood_YSpeed` | Not implemented — SoH ships `BUNNY_HOOD_FAST_AND_JUMP` enhancement already | **Equivalent via existing SoH enhancement** (just needs to be gated/set in our preset) |
| 10 | Odd Potion → child trade slot | `Statics_MoveOddPotionToChild` | `Beat1_3Butterfly.cpp:92` | **Equivalent** |
| 11 | Bazaar shop shelf swap (slot 3 → Butterfly after text 0x009C) | `Statics_PatchShop` rewrites `EnGirlA` params | `Beat1_3Butterfly.cpp:114` — detects textId 0x009C and arms `sShopHasButterfly`, **but the EnGirlA params rewrite is a stub** (`(void)ossanActor`) | **Partially implemented — the actual shelf change isn't wired yet** |
| 12 | Beta Kokiri "flying bug" quest arming | New `z_en_skj`-style actor dialogue | `Beat1_3Butterfly.cpp:162` arms `sLookingForButterfly` on close of 0x0B10 | **Equivalent (ours a flag, theirs an actor; same effect)** |
| 13 | Fire Medallion = Din's Fire on C-button; equip medallions to C | `statics/interface.c`, `z_kaleido_scope` patch, `z_player` Din's Fire path | Not implemented — `Beat2_0SagesCharm.cpp` lists as non-goals | **Missing (intentionally deferred).** This is the main "Sages' Charm" power from the original run |
| 14 | Fire Medallion thaws King Zora | Caused in-run by Din's Fire on C in Zora's Domain + `z_bg_spot07_taki` rewrite | `MedallionFire.cpp:51` — per-frame, if quest flag set, `SET_EVENTCHKINF(UNFROZE_KING_ZORA)` | **Different / simpler.** Ours is a global flag flip on possession rather than a geometry-local Din's Fire effect. Equivalent gameplay-wise; see §2 |
| 15 | Forest Medallion → Farore's Wind grant | Not an upstream mechanic | `MedallionForest.cpp:47` grants `ITEM_FARORES_WIND` on possession | **TrifoolPercent-only** |
| 16 | Water Medallion → Ice Arrows grant | Not upstream | `MedallionWater.cpp:44` grants `ITEM_ARROW_ICE` on possession | **TrifoolPercent-only** |
| 17 | Light Medallion → Light Arrows grant | Not upstream | `MedallionLight.cpp:42` grants `ITEM_ARROW_LIGHT` on possession | **TrifoolPercent-only** |
| 18 | Spirit / Shadow Medallion abilities | Not in Triforce% either | Not implemented | **N/A** |
| 19 | Custom message table | `text/messages.c` + hooked loader checks custom table first (`Statics_LoadMessage`) | Every custom textId intercepted at `OnOpenText`, built with `CustomMessage` + `LoadIntoFont()`, then `*loadFromMessageTable = false`. **No `CustomMessageManager::CreateMessage(tableID, ...)` registrations** | **Different approach, same effect for now.** See §3 — scales poorly as text count grows |
| 20 | Entrance rewiring (Potion Shop → Unicorn Fountain, Goron Shop → Triforce Room, Dampé's Hut → Ending) | `Statics_SetUpRouting` writes `gEntranceTable` at init | **Zero `gEntranceTable` writes.** We use *new* scene slots (`SCENE_BESITU`, `SCENE_TEST01`, `SCENE_SYOTES`) and reach them with custom `TfUnusedFlame` collision triggers that write `nextEntranceIndex` | **Different (ours is cleaner).** We avoid stepping on vanilla shops entirely. Trade-off: no "go through Granny's door" feel |
| 21 | Chamber of Sages scene draw config (animated water, light rays, fade-near-player) | `Patched_CoSSceneDrawConfig` in `statics/scene.c` | Not implemented (scene not yet built out) | **Missing** (blocked on Chamber of Sages `.blend` export per `trifool-asset-port-status`) |
| 22 | Cutscene terminators chained for staff-roll flyovers | Patches vanilla terminator table in `statics/scene.c` | Not implemented | **Missing** |
| 23 | Audio: Long of Time, Overture, Chamber of Sages, Triforce, Power of Goddesses, Here Together 1–3, Staff Roll, Trailer `.aseq` + banks | Patched audio DMA + static tables | Not implemented — `UnicornFountain.cpp:14` notes Overture audio deferred | **Missing** (asset-side) |
| 24 | Hair physics (`statics/hairphys.c`) | New C | Not implemented | **Missing** — relevant only once BotW Link/Zelda land |
| 25 | F3DZEX cel-shading microcode | RSP microcode replacement | Not implemented | **Hard — needs shader-side change in libultraship**, same conclusion as research doc §5 |
| 26 | Running Man race / timer hacks (`Statics_KeepTimerNegative`, time-travel-during-timer) | `z_en_mm3` rewrite + statics | Not implemented | **Missing (Beat 2 content)** |
| 27 | Gerudo guards accept child + new dialogue (`z_en_ge1/ge2`) | Actor replacements | Not implemented | **Missing (Beat 1 content)** |
| 28 | Z. Domain → L. Hylia reroute to Unicorn Fountain | Entrance 0x0560 rewired | Reached in our build via Lost Woods overworld flame trigger (`EnterFountainFromOverworld`). Zora's Domain exit itself not rerouted | **Different route — intentional** |
| 29 | Dialogue-driven onboarding at Link's house | Not in upstream | `Beat1_0Onboarding.cpp` — CVar auto-enabler via dialogue | **TrifoolPercent-only (not a port)** |
| 30 | ActorDB registrations | N/A (upstream patched overlay slots) | 3 call sites, 7 custom actors registered | **SoH-native approach (correct)** |

---

## 2. Gaps that look load-bearing

Ordered by what's likely to bite earliest.

### A. Long of Time has no consumer (SongEngine, Beat 1 end)
`SongEngine.cpp` matches the Long of Time note sequence and sets `sLastMatchedSong`, but nothing calls `TrifoolSongEngine_ConsumeMatch()` to flip Link's age and reload the entrance. Upstream's whole Beat 1 climax (Nabooru teaches it, you time-travel from the desert) depends on this. **Suggested next slice:** a `LongOfTime.cpp` that, on consumed match, mirrors `Statics_TimeTravel` — flip `gSaveContext.linkAgeOnLoad`, set `nextEntranceIndex = gSaveContext.entranceIndex`, `transitionTrigger = TRANS_TRIGGER_START`.

### B. Always-child-treatment patches (§5 of the research doc, row 8 above)
The moment Long of Time lets adult Link visit ToT, Beta Kokiri's forest, or the stones, upstream's four "always-child" patches become necessary — otherwise adult Link in ToT re-pulls the Master Sword, Sheik appears, stones animate, etc. Upstream does these as MIPS patches; the SoH-native option is a cluster of `VB_` hooks (`VB_PLAY_PULL_MASTER_SWORD_CS`, Sheik spawn conditions, demo effect spawns). **This is a known-missing low-level patch cluster.** The research doc called them out in §5.

### C. Shop shelf swap is a stub (row 11 above)
`Beat1_3Butterfly.cpp:114 TickShopPatch` finds the Bazaar `EnGirlA` but never writes its params — the butterfly never actually appears in slot 3. Upstream does this at the actor-overlay level; the SoH-equivalent is to `REGISTER_VB_SHOULD(VB_SET_SHOP_ITEM, ...)` or edit `EnGirlA_Init` params when `sShopHasButterfly && this->sohItemEntry.getItemId == vanilla-slot-3`. Currently the butterfly is unreachable through the shop flow.

### D. No `CustomMessageManager` table (row 19)
Every custom textId is caught at `OnOpenText` and loaded ad-hoc. That works for 20 messages; it scales badly past ~50 (every message pays an `OnOpenText` dispatch and a per-call `LoadIntoFont`). It also means the message table isn't visible to anything else (rando's randomized hints, debug text dumps). The SoH-idiomatic path is `CustomMessageManager::Instance->CreateMessage("TrifoolPercent", 0x0ACE, msg)` once at init, then let the vanilla text loader pick it up. **Not urgent, but worth refactoring before Beat 2/3 doubles the text count.**

### E. Medallion-on-C / Din's Fire-on-C (row 13)
This is the signature Sages' Charm power in the TAS run. `Beat2_0SagesCharm.cpp` explicitly defers it. If isaac wants the Charm to *feel* like the Charm, this one has the highest "gameplay fidelity" payoff of anything missing. Needs: a `VB_` hook / direct edit to `KaleidoScope_UpdateCursorVtx` for the medallion-grid-to-C mapping, plus `Player_ItemToItemAction` so Fire Medallion on C invokes `PLAYER_IA_DINS_FIRE`.

### F. Zora's Domain thaw is a flag flip, not Din's Fire + `taki` (row 14)
Our implementation grants the thaw on possession, upstream requires actually casting Din's Fire in the room. Gameplay-wise equivalent, but if the goal is parity with the run's shape, the thaw should trigger on Din's cast in `SCENE_ZORAS_DOMAIN` rather than globally. Low priority — it works today.

---

## 3. Over-engineered vs. the original

Nothing stands out as *doing more* than upstream in a way that worries me. The two cases worth naming:

- **Message interception at `OnOpenText`** (row 19): more code per message than a `CustomMessageManager` table would need, but it's not actor-attribute abuse — it's just a glue style that will cost us later.
- **`Setup.cpp:124` dev warps**: upstream's `Statics_TestShortcuts` has ~30 commented-out warps; ours has 3. That's a feature, not over-engineering, but when we start cutscene QA we may want to grow it toward upstream's coverage.

Of the actors you've registered, I didn't audit their internal state machines against upstream for this report — the research doc's §5 table and the standing [[prefer-engine-native-behaviors]] note (Great Fairy hover) cover that angle better than a logic-only pass can.

---

## 4. Short summary of "what's missing that's a patch, not a scene"

In priority order:

1. **Long of Time consumer** (flip age + reload) — one small `.cpp`, unblocks Beat 1 climax.
2. **Always-treat-Link-as-child cluster** (4 upstream MIPS patches → 4 VB hooks / small edits).
3. **Bazaar shop shelf-swap** finish (one `VB_` hook or EnGirlA params write).
4. **Medallion-on-C + Din's Fire-on-C** (kaleido + player edits).
5. **Overture of Sages consumer** (hijack warp path, spawn WSRCutscene).
6. **`CustomMessageManager` table refactor** (quality, not functionality).
7. **Chamber of Sages scene draw config** (water animation, light rays) — blocks on CoS `.blend` first.
8. **Audio pack** (new `.aseq` + banks) — biggest asset-side gap.
9. **Cel-shading** — hardest; same conclusion as the research doc.

Everything else upstream does that we don't is either Beat 2/3 scene/actor content (intentionally pending) or already covered by an existing SoH enhancement (Bunny Hood).
