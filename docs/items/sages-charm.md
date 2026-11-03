# Sages' Charm (a.k.a. Oracle Amulet)

A custom item added for TrifoolPercent. Upstream Triforce% refers to it as the
**Sages' Charm** in the item-get text; the showcase VOD / behind-the-scenes
talk calls it the **Oracle Amulet**. Same object.

> **Where does the Charm appear in the pause menu?** It doesn't — yet. The
> Charm is a session boolean + an item-get textbox + a set of engine patches
> that enable the medallion-on-C flow, the Fire-Medallion-as-Din's-Fire remap
> and the 3px magic-bar shift. The 24×24 C-button icon and the 128×16
> quest-status "chain" strip are vendored under `assets/textures/` but
> nothing draws them yet. Tracked as follow-up #4 below.

In Triforce% the Running Man boss hands it over after Link beats him; it is
the key that unlocks:

- equipping Medallions to the C-buttons (quest-status subscreen patch);
- Fire Medallion on a C-button acts as **Din's Fire** (player patch);
- a magic-bar visual tweak that reserves space for the medallion count.

See `docs/references/triforcepercent/code-breakdown.md` §2 (Key of Power) for
the upstream wiring.

## Slot choice

| Thing | Value | Why |
|---|---|---|
| Gameplay-item id | `TRIFOOL_ITEM_SAGES_CHARM = 0x69` | Next unused OoT quest-status / item slot above Butterfly's `0x68`. Vanilla OoT does not wire a pickup to `0x69`; TCRF lists the `0x66..0x79` range as mostly unused ia8 slots. Nothing shipped is displaced. |
| Item-get text id | `TRIFOOL_TEXT_SAGES_CHARM_GET = 0x0084` | Next free vanilla text id above Butterfly's `0x0083`. Loaded through `CustomMessage` + `OnOpenText`, same as every other TrifoolPercent text. |
| Save bit | *(session flag only, this slice)* | `sHasSagesCharm` in `Beat2_0SagesCharm.cpp`. Promotes to a `gSaveContext.itemGetInf` bit when the Beat 2 save-manager pass lands. |

Upstream repurposed `ITEM_STONE_OF_AGONY` (`0x3C`) and `GI_SAGES_CHARM` /
`GID_SAGES_CHARM` / `OBJECT_GI_SAGESCHARM` constants that only exist in the
Triforce% fork of the N64 headers. We **do not** displace Stone of Agony in
SoH because the Rumble Pak indicator still ships; we add a new id instead.

## Asset status

| Asset | State | Where |
|---|---|---|
| Model source | `.blend` vendored | `assets/model/SagesCharm/SagesCharm.blend` |
| Model `.c` | **Exported** (383 lines, 20 KB) | `assets/model/SagesCharm/object_gi_sagescharm.c` — display list `gGiSagesCharmDL` at ROM `0x6001128`, with `CharmString` + `chain` ia8 textures baked in. |
| OTR / o2r wrapper | **Pending** | Lands in the next `TrifoolPercent.o2r` build when the Fast64 pipeline covers the SagesCharm object. Same follow-up as the Butterfly's 3D asset. |
| C-button icon (24×24 rgba32) | **Vendored** | `assets/textures/sages_charm_24.rgba32.{png,bin}` — ready to blit on a C button or draw as an overlay. Not yet referenced by `soh/Enhancements/TrifoolPercent/` code. |
| Quest-status "chain" strip (128×16 ia4) | **Vendored** | `assets/textures/item_sages_charm.ia4.{png,bin}` — upstream draws it across the medallion row on the Quest Status page when the Charm is held. Not yet referenced by `soh/Enhancements/TrifoolPercent/` code. |
| Dialogue-box inline icon | **Not needed** | The item-get card renders through the vanilla textbox fanfare path; once the GetItemEntry lands, `CustomMessageManager::CreateGetItemMessage` stamps the item-icon header automatically. |

## Scope of Beat 2.0 (merged in this PR)

- Session flag + exports (`TrifoolBeat_HasSagesCharm`, `TrifoolBeat_GrantSagesCharm`).
- Item id + text id defined in `TrifoolPercent.h`.
- CustomMessage item-get card: "You got the Sages' Charm! You can now equip
  Medallions to the C-buttons. The Fire Medallion acts as Din's Fire."
- Menu toggle (`CVAR_TRIFOOL_SAGES_CHARM`) + a one-shot "Grant Sages' Charm
  (debug)" pulse so follow-up slices can be exercised without the Running
  Man boss.
- Reset on `OnLoadGame`.

## Status of follow-up slices

The Setup preset (`CVAR_TRIFOOL_SETUP_PRESET`) now auto-grants the Sages' Charm when
`CVAR_TRIFOOL_SAGES_CHARM` is also on, so dev warps land in a state where the Charm
flag reads true without having to tap the debug grant each run.

The six medallion beta-ability slices (follow-up 8 below) now ship:

- **Fire** = Din's Fire (native) + melts Zora's Domain — PR #52.
- **Forest** = Farore's Wind — PR #53.
- **Water / Ice** = Ice Arrows, **Light** = Light Arrows — PR #54.
- **Spirit** (Navi free-fly) and **Shadow** (darkness stealth) still owed.

Each of those PRs grants the vanilla elemental item when the medallion is held, so the
ability is reachable from the vanilla equip slots and does not block on the C-button
work below. See `docs/references/beta-medallions.md`.

Follow-ups still open (status refreshed 2026-11-02):

1. **Medallion-on-C enabler.** ✅ **Done** — `Beat2_1MedallionOnC.cpp` + the
   weak-symbol patch `patches/shipwright/kaleido-equip-medallions-to-c.patch`.
   Our hook reads C-press directly and calls `KaleidoScope_SetupItemEquip`,
   so unlike upstream we do not need to toggle `sSubscreenButtonStates`.
   Enable SoH's `gEnhancements.PauseAnyCursor` → "Always" if you want the
   Quest Status cursor to park on any slot (not required for medallions,
   which are already cursor-reachable in vanilla).
2. **Fire Medallion as Din's Fire.** ✅ **Done** —
   `TrifoolBeat_MapMedallionItemAction` in `Beat2_1MedallionOnC.cpp` +
   `patches/shipwright/player-medallion-item-action.patch`.
3. **Magic-bar visual tweak.** ✅ **Done** — `TrifoolBeat_MagicBarYOffset`
   in `Beat2_1MedallionOnC.cpp` + `patches/shipwright/magic-bar-sages-charm-offset.patch`.
4. **24×24 C-button icon.** Asset vendored (`sages_charm_24.rgba32`). Still
   needs a draw hook: nothing in `soh/Enhancements/TrifoolPercent/` references
   it yet, so the Charm itself is not rendered on any C button or in the
   pause menu. Likely approach: a weak-symbol hook at the end of
   `KaleidoScope_DrawQuestStatus` that draws the 24×24 overlay (and/or the
   128×16 "chain" strip across the medallion row) when `TrifoolBeat_HasSagesCharm()`.
5. **GetItemEntry via `ItemTableManager`.** Wire `gGiSagesCharmDL` through a
   `GetItemEntry {.getItemId = ..., .objectId = ..., .gid = ...}` and the
   `object_gi_sagescharm.o2r` resource path, so chests and NPCs can deliver
   the Charm through the vanilla En_Box pipeline. Blocked on the next
   `TrifoolPercent.o2r` build (Fast64 pipeline covering the SagesCharm object).
6. **Running Man boss handoff.** When `BossRunningMan` is ported, its
   "deliver reward" state calls `TrifoolBeat_GrantSagesCharm()` and starts
   `TRIFOOL_TEXT_SAGES_CHARM_GET` through En_Box.
7. **Save persistence.** Promote `sHasSagesCharm` to a `gSaveContext` bit
   (unused `itemGetInf` bit, or an SoH custom-save section if one lands).
8. **Medallion beta abilities.** Each medallion's upstream beta ability
   (Light Medallion = teleport, Shadow Medallion = darken-screen, etc.) is
   a separate slice that gates on both this flag and the specific medallion
   being equipped to a C-button.

Related reference material:
- `docs/references/triforcepercent/code-breakdown.md` §2 for the full
  upstream behavior.
- `docs/references/soh-engine-capabilities.md` §2 for the
  `ItemTableManager` + `CustomMessageManager` + `En_Box` pipeline.
- `scripts/statics/interface.c` for the upstream patch set (medallions on
  C, HasSagesCharm predicate, magic-bar patches, model DL wiring).
