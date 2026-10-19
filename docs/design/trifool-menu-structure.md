# TrifoolPercent menu structure

Design spec for the SoH menu tab that houses every TrifoolPercent toggle, cheat,
troubleshooting tool and config knob. This is the shape we're building toward;
not every row ships at once.

## Where it lives

A new top-level tab **TrifoolPercent** sitting next to `Settings` / `Enhancements`
/ `Dev Tools` / `Network` / `Randomizer`. Registered from
`soh/Enhancements/TrifoolPercent/menu/TrifoolMenu.cpp` with:

```cpp
SohGui::mSohMenu->AddMenuEntry("TrifoolPercent", CVAR_SETTING("Menu.TrifoolPercentSidebarSection"));
```

## Sidebars

Insertion order is the on-screen order. **General** is first so it's the
default view when the tab opens.

| # | Sidebar          | Purpose                                                  | Status  |
|---|------------------|----------------------------------------------------------|---------|
| 1 | General          | Master toggles for the whole experience                  | ships now (holds today's checkboxes) |
| 2 | Scenes & Rooms   | Per-scene enables + overworld entrances                  | stubbed |
| 3 | Beats & Progression | Per-beat toggles, song/medallion engine, Sages' Charm | stubbed |
| 4 | Cutscenes        | Per-cutscene toggles + skip/replay                       | stubbed |
| 5 | Music & Audio    | Music engine, per-area overrides                         | stubbed (empty until music engine lands) |
| 6 | Quality of Life  | TrifoolPercent-specific QoL                              | stubbed |
| 7 | Cheats           | Grant Triforce, grant medallions, warp to beat           | stubbed |
| 8 | Inventory & Flags | Inspect/edit TrifoolPercent event flags & state         | stubbed |
| 9 | Dev / Debug      | Dev-warps, actor spawn, log toggles, test actors         | existing dev CVars move here |

---

## 1. General

| Label                                       | Widget   | CVar                                 | Default | Notes |
|---------------------------------------------|----------|--------------------------------------|---------|-------|
| Experience master switch                    | Checkbox | `CVAR_TRIFOOL_EXPERIENCE_ENABLED` *(new)* | on | Kill-switch for the whole mod. When off, every other toggle is no-op. |
| Triforce Room                               | Checkbox | `CVAR_TRIFOOL_TRIFORCE_ROOM`         | on      | existing |
| Test Flame by Link's House (hex hub entry)  | Checkbox | `CVAR_TRIFOOL_TEST_ENTRANCE`         | on      | existing |
| Unicorn Fountain                            | Checkbox | `CVAR_TRIFOOL_UNICORN_FOUNTAIN`      | on      | existing |
| Unicorn Fountain entry cutscene             | Checkbox | `CVAR_TRIFOOL_FOUNTAIN_ENTRY_CS`     | on      | existing |
| Overworld entrance to Unicorn Fountain      | Checkbox | `CVAR_TRIFOOL_FOUNTAIN_OVERWORLD`    | off     | existing |
| Door of Time opens for adult Link           | Checkbox | `CVAR_TRIFOOL_DOT_ADULT`             | off     | existing — may move to Scenes & Rooms once that sidebar lands |
| Mimic Chest in Test Room                    | Checkbox | `CVAR_TRIFOOL_MIMIC_CHEST`           | off     | existing — may move to Dev / Debug once that sidebar lands |

> On ship-day the General sidebar is exactly the 7 existing checkboxes plus the
> new master switch. Reorganization into Scenes/Rooms vs Dev/Debug comes with
> the follow-up PR that introduces those sidebars.

## 2. Scenes & Rooms

Everything that enables a custom scene, room, or warp point.

| Label                              | Widget   | CVar                              | Default | Notes |
|------------------------------------|----------|-----------------------------------|---------|-------|
| Triforce Room                      | Checkbox | `CVAR_TRIFOOL_TRIFORCE_ROOM`      | on      | mirrors General (eventually General demotes to the master switch only) |
| Unicorn Fountain                   | Checkbox | `CVAR_TRIFOOL_UNICORN_FOUNTAIN`   | on      | |
| Hex hub Test Room                  | Checkbox | `CVAR_TRIFOOL_TEST_ROOM`          | on      | existing |
| Lost Woods path addition           | Checkbox | `CVAR_TRIFOOL_LOST_WOODS_PATH`    | on      | existing |
| Test Flame by Link's House         | Checkbox | `CVAR_TRIFOOL_TEST_ENTRANCE`      | on      | |
| Overworld entrance to Unicorn Fountain | Checkbox | `CVAR_TRIFOOL_FOUNTAIN_OVERWORLD` | off | |
| Door of Time opens for adult Link  | Checkbox | `CVAR_TRIFOOL_DOT_ADULT`          | off     | world patch, not strictly a scene — lives here for lack of a better home |

## 3. Beats & Progression

The quest-logic layer: beats, songs, medallions, Sages' Charm.

| Label                              | Widget   | CVar                                      | Default | Notes |
|------------------------------------|----------|-------------------------------------------|---------|-------|
| Beat 1.0 — Kokiri onboarding       | Checkbox | `CVAR_TRIFOOL_ONBOARDING`                 | on      | existing |
| Beat 1.1 — Beta Kokiri             | Checkbox | `CVAR_TRIFOOL_BETA_KOKIRI`                | on      | existing |
| Beat 1.3 — Butterfly               | Checkbox | `CVAR_TRIFOOL_BUTTERFLY`                  | on      | existing |
| Beat 2.0 — Sages' Charm            | Checkbox | `CVAR_TRIFOOL_SAGES_CHARM`                | on      | existing |
| Sages' Charm grant pulse FX        | Checkbox | `CVAR_TRIFOOL_SAGES_CHARM_GRANT_PULSE`    | on      | existing |
| Beat 2.1 — Medallion-on-C          | Checkbox | `CVAR_TRIFOOL_MEDALLION_ON_C`             | on      | existing |
| ├ Medallion of Fire                | Checkbox | `CVAR_TRIFOOL_MEDALLION_FIRE`             | on      | existing |
| ├ Medallion of Forest              | Checkbox | `CVAR_TRIFOOL_MEDALLION_FOREST`           | on      | existing |
| ├ Medallion of Light               | Checkbox | `CVAR_TRIFOOL_MEDALLION_LIGHT`            | on      | existing |
| └ Medallion of Water               | Checkbox | `CVAR_TRIFOOL_MEDALLION_WATER`            | on      | existing |
| Song engine (Overture-of-Sages framework) | Checkbox | `CVAR_TRIFOOL_SONG_ENGINE`         | on      | existing |

## 4. Cutscenes

Every custom cutscene gets a toggle (play / skip) and a "replay now" button
during gameplay for troubleshooting.

| Label                              | Widget   | CVar                                      | Default | Notes |
|------------------------------------|----------|-------------------------------------------|---------|-------|
| "Long of Time" intro cutscene      | Checkbox | `CVAR_TRIFOOL_LONG_OF_TIME`               | on      | existing |
| Long of Time — age flip variant    | Checkbox | `CVAR_TRIFOOL_LONG_OF_TIME_AGE_FLIP`      | on      | existing |
| Unicorn Fountain entry pan         | Checkbox | `CVAR_TRIFOOL_FOUNTAIN_ENTRY_CS`          | on      | |
| Triforce Room prototype cutscene   | Checkbox | `CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS`     | on      | existing |
| Sacred Realm cutscene chain        | Checkbox | `CVAR_TRIFOOL_SACRED_REALM_CHAIN`         | on      | existing |
| Ending (Triforce get)              | Checkbox | `CVAR_TRIFOOL_ENDING`                     | on      | existing |
| Staff roll chain                   | Checkbox | `CVAR_TRIFOOL_STAFF_ROLL_CHAIN`           | on      | existing |
| ── Replay buttons ──               |          |                                           |         | |
| Replay Long of Time                | Button   | fires in-game cutscene                    | —       | dev convenience; disabled when no save loaded |
| Replay Unicorn Fountain arrival    | Button   | fires in-game cutscene                    | —       | |
| Replay Triforce get                | Button   | fires in-game cutscene                    | —       | |

## 5. Music & Audio

Empty on ship-day. Stub sidebar reserved for when the music engine lands.
Planned shape:

| Label                                   | Widget   | CVar                                 | Default |
|-----------------------------------------|----------|--------------------------------------|---------|
| Music engine                            | Checkbox | `CVAR_TRIFOOL_MUSIC_ENGINE` *(new)*  | on |
| Replace Unicorn Fountain BGM            | Checkbox | `CVAR_TRIFOOL_MUSIC_FOUNTAIN` *(new)* | on |
| Overture of Sages — custom arrangement  | Checkbox | `CVAR_TRIFOOL_MUSIC_OVERTURE` *(new)* | on |
| Triforce Room BGM                       | Checkbox | `CVAR_TRIFOOL_MUSIC_TRIFORCE_ROOM` *(new)* | on |
| Master music volume override            | Slider   | `CVAR_TRIFOOL_MUSIC_VOLUME` *(new)` | 1.0 |

## 6. Quality of Life

Mod-specific QoL. Not meant to overlap with SoH's own Enhancements → QoL.

| Label                                   | Widget   | CVar                                      | Default |
|-----------------------------------------|----------|-------------------------------------------|---------|
| Fast text in TrifoolPercent scenes      | Checkbox | `CVAR_TRIFOOL_QOL_FAST_TEXT` *(new)*      | off |
| Skip to next beat on cutscene skip      | Checkbox | `CVAR_TRIFOOL_QOL_SKIP_ADVANCES_BEAT` *(new)* | off |
| Show beat progress HUD                  | Checkbox | `CVAR_TRIFOOL_QOL_BEAT_HUD` *(new)*       | off |
| Auto-save on beat completion            | Checkbox | `CVAR_TRIFOOL_QOL_AUTOSAVE_BEATS` *(new)* | on |

## 7. Cheats

Gates behind SoH's existing "cheats enabled" check so they can't be
accidentally triggered.

| Label                                   | Widget   | CVar                                      | Default |
|-----------------------------------------|----------|-------------------------------------------|---------|
| Give Triforce (sets flag + inventory)   | Button   | —                                         | — |
| Grant all four medallions-on-C          | Button   | —                                         | — |
| Grant Sages' Charm                      | Button   | —                                         | — |
| Warp to beat…                           | Combo    | `CVAR_TRIFOOL_SETUP_PRESET`               | 0 (existing) |
| Unlock all cutscenes replay             | Button   | —                                         | — |
| Force Door of Time open                 | Button   | writes `EVENTCHKINF_OPENED_DOOR_OF_TIME`  | — |

## 8. Inventory & Flags

Troubleshooting view: what state does the game think TrifoolPercent is in?
Each row shows the current value and a toggle/edit control.

| Label                                   | Widget   | Source                                    |
|-----------------------------------------|----------|-------------------------------------------|
| Current beat                            | Readonly | computed from eventChkInf + custom flags  |
| Sages' Charm obtained                   | Toggle   | custom flag                                |
| Overture of Sages learned               | Toggle   | custom flag                                |
| Medallion-on-C: Fire / Forest / Light / Water | Toggles | custom flags                        |
| Long of Time played (child / adult)     | Toggles  | custom flags                               |
| Triforce obtained                       | Toggle   | custom flag                                |
| Reset all TrifoolPercent flags          | Button   | — (confirmation dialog)                    |
| Export flag dump to clipboard           | Button   | — (for bug reports)                        |

## 9. Dev / Debug

Developer convenience + the existing debug CVars.

| Label                                   | Widget   | CVar                                      | Default |
|-----------------------------------------|----------|-------------------------------------------|---------|
| Dev inputs (shortcut key combos)        | Checkbox | `CVAR_TRIFOOL_DEV_INPUTS`                 | off (existing) |
| Pause-menu debug inventory              | Checkbox | `CVAR_TRIFOOL_PAUSE_DEBUG_INV`            | off (existing) |
| Setup preset (dev-warp to beat N)       | Combo    | `CVAR_TRIFOOL_SETUP_PRESET`               | 0 (existing) |
| Mimic Chest test spawn                  | Checkbox | `CVAR_TRIFOOL_MIMIC_CHEST`                | off (existing) |
| Dev-warp: Triforce Room                 | Button   | —                                         | — |
| Dev-warp: Unicorn Fountain              | Button   | —                                         | — |
| Dev-warp: Hex hub Test Room             | Button   | —                                         | — |
| Spawn Beta Great Fairy here             | Button   | —                                         | — |
| Spawn Unused Flame here                 | Button   | —                                         | — |
| Log TrifoolPercent events to console    | Checkbox | `CVAR_TRIFOOL_LOG_EVENTS` *(new)*         | off |

---

## File layout

```
soh/Enhancements/TrifoolPercent/menu/
  TrifoolMenu.cpp              ← AddMenuEntry + wire each sidebar
  TrifoolMenu.h                ← shared path helper, CVar names, forward decls
  TrifoolMenuGeneral.cpp       ← ships now
  TrifoolMenuScenes.cpp        ← stub (sidebar registered, body empty)
  TrifoolMenuBeats.cpp         ← stub
  TrifoolMenuCutscenes.cpp     ← stub
  TrifoolMenuMusic.cpp         ← stub
  TrifoolMenuQoL.cpp           ← stub
  TrifoolMenuCheats.cpp        ← stub
  TrifoolMenuInventory.cpp     ← stub
  TrifoolMenuDebug.cpp         ← stub
```

A "stub" sidebar registers its sidebar entry (so the row shows in the nav
list) and renders a single greyed-out "Coming soon" label, so breakage of the
nav wiring is visually obvious even before content lands.

## CVars introduced by this PR

Only one, and only if we agree it's wanted:

- `CVAR_TRIFOOL_EXPERIENCE_ENABLED` — master kill-switch.

Everything else is a rename-free move of existing CVars under the new tab.

## Smoke test

After this PR, with no save-file changes:

1. New **TrifoolPercent** tab appears next to the five SoH tabs.
2. Nine sidebars listed in order: General, Scenes & Rooms, Beats & Progression,
   Cutscenes, Music & Audio, Quality of Life, Cheats, Inventory & Flags,
   Dev / Debug.
3. Clicking any stub sidebar shows the "Coming soon" placeholder.
4. The **General** sidebar shows exactly the 7 existing checkboxes (plus the
   master switch, if we add it) with matching tooltips and defaults.
5. The old `Enhancements → TrifoolPercent` sidebar is gone.
6. Toggling any General widget still gates its scene/feature as before.
7. Save/reload: toggle states persist (CVar names unchanged).

If any step fails, blame points at a single `Register...` call in the file
corresponding to the failing sidebar.
