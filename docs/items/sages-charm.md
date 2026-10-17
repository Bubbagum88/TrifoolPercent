# Sages' Charm (a.k.a. Oracle Amulet)

A custom item added for TrifoolPercent. Upstream Triforce% refers to it as the
**Sages' Charm** in the item-get text; the showcase VOD / behind-the-scenes
talk calls it the **Oracle Amulet**. Same object.

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
| C-button icon (24×24 ia8) | **Missing** | Upstream shipped a new 24px icon; no source asset in the repo yet. Needs Isaac's Blender / texture pipeline to export the icon alongside the next o2r build. |
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

## Scope of follow-up slices (not in this PR)

1. **Medallion-on-C enabler.** Allow the quest-status subscreen to pick a
   medallion and equip it to a C-button. Upstream patches `z_kaleido_scope`
   and `sSubscreenButtonStates`; in SoH this is a VB hook on the kaleido
   C-button gate (`REGISTER_VB_SHOULD(VB_KALEIDO_...)` or a direct kaleido
   edit). Gated on `TrifoolBeat_HasSagesCharm()`.
2. **Fire Medallion as Din's Fire.** When Link presses a C-button that holds
   `ITEM_FIRE_MEDALLION`, trigger Din's Fire. Upstream patches
   `Player_ItemToItemAction`; in SoH this is a Player VB hook. Also gated
   on `TrifoolBeat_HasSagesCharm()`.
3. **Magic-bar visual tweak.** Shift the magic bar down 3px when the Charm
   is held (upstream `Patched_MagicBarSetup/Y1/Loads`). Visual only.
4. **24×24 C-button icon.** Register once the asset exists.
5. **GetItemEntry via `ItemTableManager`.** Wire `gGiSagesCharmDL` through a
   `GetItemEntry {.getItemId = ..., .objectId = ..., .gid = ...}` and the
   `object_gi_sagescharm.o2r` resource path, so chests and NPCs can deliver
   the Charm through the vanilla En_Box pipeline. Lands with the next
   `TrifoolPercent.o2r` build.
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
