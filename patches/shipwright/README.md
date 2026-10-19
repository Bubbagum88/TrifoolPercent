# Shipwright engine patches

Surgical patches applied to `third_party/Shipwright` *after* `tools/setup.sh` clones it and *before* `tools/build_soh.sh` runs CMake. Each patch is anchored by exact-context diff, so a drifting upstream commit fails loudly (patch rejects) rather than silently miscompiles.

Scope. Only edits that genuinely cannot live in the `soh/Enhancements/TrifoolPercent/` module — i.e. the hook point is `static` in an overlay translation unit, or the behaviour is a drawing tweak with no existing `VB_` / `GameInteractor` hook. Everything else stays module-side.

## Current patches (Sages' Charm / Beat 2.1)

| File | Shipwright file | What it does |
|---|---|---|
| `kaleido-equip-medallions-to-c.patch` | `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_collect.c` | Injects one `TrifoolBeat_UpdateQuestEquipHook(play)` call at the top of `KaleidoScope_DrawQuestStatus`. The hook body (module-side) decides whether a C-button press on a medallion fires `KaleidoScope_SetupItemEquip`, gated on Sages' Charm held. |
| `player-medallion-item-action.patch` | `soh/src/overlays/actors/ovl_player_actor/z_player.c` | Adds one `TrifoolBeat_MapMedallionItemAction(item)` call-out in `Player_ItemToItemAction` so Fire Medallion-on-C fires `PLAYER_IA_DINS_FIRE`. Rides next to the existing `VB_ITEM_ACTION_BE_NONE` branch. |
| `magic-bar-sages-charm-offset.patch` | `soh/src/code/z_parameter.c` | Shifts the magic bar Y by `+3` while the Charm is held, mirroring upstream `Patched_MagicBarSetup`. Reserves vertical room for the eventual medallion-count display. |

Each patch is a self-contained unified diff. They are expected to apply cleanly against `SHIPWRIGHT_COMMIT` in `tools/setup.sh`; when that commit is bumped and a patch rejects, re-anchor by hand and commit the updated `.patch` in the same change.

## Linkage to the module

Module-side glue lives in `soh/Enhancements/TrifoolPercent/Beat2_1MedallionOnC.cpp`:

- `extern "C" void TrifoolBeat_UpdateQuestEquipHook(PlayState* play)` — patch #1 entry.
- `extern "C" s8 TrifoolBeat_MapMedallionItemAction(s32 item, s8 fallback)` — patch #7 entry.
- `extern "C" s16 TrifoolBeat_MagicBarYOffset(void)` — patch #5 entry.

All three return a zero-effect value when `CVAR_TRIFOOL_MEDALLION_ON_C` is off or the Charm isn't held, so an engine binary that is patched but has the mod disabled behaves exactly like vanilla.

## Not patched here (follow-ups)

- `gDrawItemTable[GID_SAGES_CHARM]` and `relocGetItemTable[GI_SAGES_CHARM-1]` wiring — module route exists via `ItemTableManager::AddItemEntry`, lands when `TrifoolPercent.o2r` ships `object_gi_sagescharm.o2r` (same follow-up as the Butterfly 3D model). Tracked in `docs/items/sages-charm.md`.
- 24×24 C-button icon for the Charm itself — needs the icon asset exported alongside the next o2r build.
- Running Man boss handoff — lands when `BossRunningMan` is ported.
