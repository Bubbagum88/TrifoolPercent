# Beta medallion abilities

The six Sage Medallions in retail OoT are quest trophies: they sit in the Quest Status screen, mark boss completions, and feed one cutscene (Zelda handing Link the Light Arrows). In the Space World 1997 build they were **usable items** with their own powers, equippable to C in a slot the Sages' Charm unlocked. This doc is the running inventory of what each one did (or was theorised to do), so the TrifoolPercent restoration can land each ability against a cited source rather than a guess.

Written against the Jan 2021 Space World '97 leak (not the Aug 2020 "gigaleak") and the 1997 Miyamoto interview that preceded it. Translations come from the leaked ROM's unused text, dumped by MrCheeze and others.

**Convention.** The retail names (Forest, Water, Spirit, Shadow) sit alongside the beta names (Wind, Ice, Soul, Darkness). Where only the beta name appears in a source quote, both are given once at the top of the entry.

---

## Fire Medallion

**Beta effect:** A Din's Fire–style AoE spell. Casting it burns things in a radius around Link.

- Translated text and MrCheeze's own notes describe the Fire Medallion as functioning like the retail Din's Fire. The retail Din's Fire inherited this behaviour; the beta would have delivered it via the medallion directly, consuming its own magic charge. ([VGC overview](https://www.videogameschronicle.com/news/a-zelda-64-beta-version-has-been-discovered-and-fans-are-pulling-it-apart/), [Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/))
- Triforce% extends this with a specific gameplay rider: casting the Fire Medallion thaws Zora's Domain, which in the Triforce% route unlocks the Key-of-Wisdom path through the Unicorn Fountain. The Triforce% repo wires this in `actor/z_bg_spot07_taki` (rewritten) plus a Zora's Domain scene/room patch. See [triforcepercent/code-breakdown.md §3.1](triforcepercent/code-breakdown.md).

**Status in TrifoolPercent:** First slice landed in [PR #52](https://github.com/Bubbagum88/TrifoolPercent/pull/52). While the toggle is on and the player holds the Fire Medallion, Zora's Domain is treated as thawed (`EVENTCHKINF_UNFROZE_KING_ZORA`). The AoE-flame-on-C side waits for the engine-side Sages'-Charm PR.

## Water Medallion ( = Ice Medallion in beta )

**Beta effect:** Puts out fire. Walking up to a lit torch (or similar fire-element world object) with the medallion active extinguishes it. One reading of the Majora's Mask leftover text pairs it with Ice Arrows as the on-use effect, so the "cold projectile" reading is defensible as an alternate interpretation but the extinguisher reading is primary.

- Zelda Dungeon's roundup of the leaked text: *"the Ice/Water Medallion would have extinguished fires."* ([Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/))
- TCRF's Majora's Mask Unused Items page carries OoT leftover entries for Wind / Fire / Ice Medallions whose on-use effects list Fire / Ice / Light arrows respectively — this is where the "Ice Arrows" reading originates. Treat it as the earlier draft of the ability, superseded by the leaked text's "extinguishes fire" ([TCRF](https://tcrf.net/The_Legend_of_Zelda:_Majora%27s_Mask/Unused_Items)).
- MrCheeze's build note separately pairs an "Ice Medal" with the "Waltz of Water" song in the beta.

**Status in TrifoolPercent:** Spell-proxy slice in [PR #54](https://github.com/Bubbagum88/TrifoolPercent/pull/54) (grants Ice Arrows). The passive torch-extinguish behaviour needs an engine-side hook on `Obj_Syokudai` and lands in a separate PR.

## Forest Medallion ( = Wind Medallion in beta )

**Beta effect:** Warp spell. Equivalent to retail Farore's Wind, or to the Sheik warp songs — the player sets a warp point, casting warps them back to it.

- The leaked text's warp description, consistent across every roundup: *"the Wind/Forest Medallion would've allowed Link to warp much like Sheik's songs do in the final build."* ([Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/))
- Miyamoto in the 1997 preview interview grouped the medallions as spells of *"fire, wind, darkness, and light"* — the Wind element confirms the warp framing predates the final retail Forest/Nature rename.
- Triforce% leaves the warp to vanilla Farore's Wind and only wires the medallion-to-C equip. See [triforcepercent/code-breakdown.md §5](triforcepercent/code-breakdown.md).

**Status in TrifoolPercent:** First slice landed in [PR #53](https://github.com/Bubbagum88/TrifoolPercent/pull/53). While the toggle is on and the player holds the Forest Medallion, Farore's Wind is placed in its inventory slot so it's equippable to C.

## Spirit Medallion ( = Soul Medallion in beta )

**Beta effect:** Link transforms into Navi and flies around in first-person. The leftover visual-effect data for this spell was later reused for Nayru's Love.

- Zelda Dungeon's summary, verbatim: *"Soul Medallion … would have let players transform into the fairy Navi and fly around the environment."* MrCheeze's follow-up quote: *"This is the same effect as a spell in Zelda II, but way more complicated to implement in a 3D game"* — he suggests this is why the ability was cut and its effect redirected to Nayru's Love in the retail game. ([Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/), [VGC](https://www.videogameschronicle.com/news/a-zelda-64-beta-version-has-been-discovered-and-fans-are-pulling-it-apart/))
- The AoL (Zelda II) analogue is the **Fairy spell** — Link transforms into a fairy, flies at reduced speed, can enter small gaps, cannot attack. Reasonable implementation target for the restoration: camera to first-person, swap the player skeleton to a Navi-style billboard sprite, fly-mode physics (no gravity, directional thrust from the stick), continuous magic drain.

**Status in TrifoolPercent:** Not yet implemented. No clean vanilla inventory-grant proxy exists for "turn into Navi"; the ability requires player state + camera + physics manipulation. Flagged as the highest-complexity medallion of the six.

## Shadow Medallion ( = Darkness / Dark Medallion in beta )

**Beta effect:** Stealth. Enemies cannot see Link while the medallion is active. One source compares it directly to the Stone Mask introduced in Majora's Mask.

- Zelda Dungeon's roundup: *"the Dark/Shadow Medallion would have hidden Link from enemies like the Stone Mask."* ([Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/))
- Majora's Mask shipped this behaviour on the Stone Mask: equipping it causes enemies and NPCs that would normally detect Link to ignore him. Treat the Stone Mask code path as the design reference for the restoration — enemy AI target hook on each enemy actor, filtered by "player is wearing the stealth marker."
- Implementation note: the vanilla OoT enemy actor targeting path is per-actor (each enemy's state machine checks line-of-sight itself), so there is no single flag to flip; the restoration needs an `OnActorUpdate`-style hook that forces the relevant enemy state away from Chase/Aggro while the stealth is active.

**Status in TrifoolPercent:** Not yet implemented. No clean vanilla inventory-grant proxy exists; waiting on the engine-side enemy-target hook.

## Light Medallion

**Beta effect:** Ambiguous between two readings; both are attested in the leak coverage.

1. **Illuminates darkness.** The leaked unused text suggests the Light Medallion would have *"lit up dark rooms"* — i.e. a passive ambient-light override that turns the dark setup of a scene (dark Shadow Temple corridors, dark Bottom of the Well variants) into its lit setup. This is the reading most commentary leans on and the one closest to the "elemental utility" shape of the other five medallions. ([Zelda Dungeon](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/))
2. **Light Arrow–style attack.** MrCheeze himself has separately described the Light Medallion as *"an attack that works similarly to Light Arrows in the final game, but with a larger range."* The leaked unused arrow textures include a white "Light arrow" that was later recoloured yellow for retail — hinting the beta had a dedicated Light projectile tied to the medallion.

Both readings are plausible, and the two could coexist (passive illumination + active light bolt). The restoration should ship the illumination reading first (it is the simpler, more consistently-reported one) and the arrow is already covered by vanilla retail Light Arrows.

**Status in TrifoolPercent:** Spell-proxy slice in [PR #54](https://github.com/Bubbagum88/TrifoolPercent/pull/54) (grants Light Arrows — reading 2). The passive-illumination reading (1) needs engine-side `gPlayState->envCtx` edits and lands in a separate PR.

---

## Summary table

| Medallion | Beta name | Beta ability | Vanilla analogue the proxy slice uses | Direct behaviour still owed |
|---|---|---|---|---|
| Fire | Fire | AoE flame (Din's Fire–shaped) | — (abilities already live natively as Din's Fire; event-flag rider lands the Zora's Domain thaw) | AoE cast tied to the medallion C-slot specifically |
| Water | Ice | Puts out fire | Ice Arrows | Passive torch-extinguish on contact (`Obj_Syokudai` hook) |
| Forest | Wind | Warp spell | Farore's Wind | — (vanilla FW is the ability) |
| Spirit | Soul | Turn into Navi, free-fly | — (no clean analogue) | Full player/camera/physics transformation |
| Shadow | Darkness | Stealth, enemies ignore Link | — (no clean analogue) | Per-enemy AI target hook |
| Light | Light | Illuminates darkness (or Light-Arrow–style attack) | Light Arrows | Passive ambient-light override (`envCtx` edits) |

## Sources

- [Zelda Dungeon — "Discovered Beta Version of Ocarina of Time Offers a Glimpse of What Could've Been"](https://www.zeldadungeon.net/discovered-beta-version-of-ocarina-of-time-offers-a-glimpse-of-what-couldve-been/)
- [VideoGamesChronicle — "A Zelda 64 beta version has been discovered"](https://www.videogameschronicle.com/news/a-zelda-64-beta-version-has-been-discovered-and-fans-are-pulling-it-apart/)
- [TCRF — Majora's Mask Unused Items (OoT leftover medallion entries)](https://tcrf.net/The_Legend_of_Zelda:_Majora%27s_Mask/Unused_Items)
- [TCRF — OoT Late 1997 Overdump Unused Textures](https://new.tcrf.net/Proto:The_Legend_of_Zelda:_Ocarina_of_Time/Late_1997_Overdump/Unused_Textures)
- [Zelda Universe — "Zelda's Study: The Unused Utility of the Sage Medallions"](https://zeldauniverse.net/2022/04/12/zeldas-study-the-unused-utility-of-the-sage-medallions/)
- [MrCheeze on X — 0.9 build note](https://x.com/MrCheeze_/status/1351672856225214466)
- [triforcepercent/code-breakdown.md](triforcepercent/code-breakdown.md) — the Triforce% hook points this repo ports from
