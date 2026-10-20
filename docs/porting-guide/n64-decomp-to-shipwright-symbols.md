# Porting N64 OoT Decomp Code to Shipwright: Symbol Name Translation

If you're porting code from the N64 Ocarina of Time decomp project
([zeldaret/oot](https://github.com/zeldaret/oot)) into a Ship of Harkinian
(Shipwright) mod, you will hit a wall of compile errors about undeclared
identifiers like `func_80034A14`, `Actor_IsTalking`, or `PLAYER_AP_BOTTLE`.
This document explains why that happens and how to translate those symbols
to their Shipwright equivalents.

## The problem

The N64 OoT decomp uses two kinds of symbol names:

1. **Named symbols** — functions and fields the decomp community has
   reverse-engineered and given a descriptive name (e.g. `Actor_Kill`,
   `Player_PlaySfx`).
2. **Placeholder symbols** — functions that haven't been named yet, so the
   decomp refers to them by their N64 ROM virtual address, e.g.
   `func_80034A14`, `func_80078884`. The `0x80XXXXXX` range is where the
   N64 OoT code lived in RAM.

Shipwright is a native PC port that **tracks the decomp but is not identical
to it**. In Shipwright:

- The address-derived names mean nothing — the code runs on a PC, not an N64,
  and the memory map is completely different.
- Some `func_80XXXXXX` placeholders have been given human-readable names by
  the Shipwright maintainers that the upstream decomp has not yet adopted.
- Some symbols have been renamed on both sides but at different times, so
  the names may differ.
- Enum prefixes, struct field names, and entrance IDs have been reworked
  for clarity in Shipwright.

The result: code that compiles cleanly against the N64 decomp will fail
to compile against Shipwright until every renamed symbol is translated.

## Why it happens

Three independent forces produce the mismatch:

1. **Decomp placeholder names are temporary.** The decomp assigns
   `func_80XXXXXX` names from the ROM symbol table and renames them as
   contributors reverse-engineer each function. Any port that pins to a
   particular decomp commit freezes a snapshot of this renaming progress.
2. **Shipwright's renames are independent.** Shipwright maintainers rename
   functions to clarify PC-side code without waiting for upstream decomp.
   Sometimes Shipwright is ahead (named earlier), sometimes behind
   (upstream renamed first).
3. **The memory address space is gone.** On a PC build there is no
   `0x80XXXXXX` region, so the raw-address naming scheme loses its
   justification entirely. Shipwright treats `func_80XXXXXX` as a legacy
   placeholder, not an address.

## How to look up the correct name

Shipwright's [`soh/include/functions.h`](https://github.com/HarbourMasters/Shipwright/blob/develop/soh/include/functions.h)
is the authoritative function declaration list. When a build fails with
`'func_80XXXXXX' was not declared in this scope`, this is where you look
first.

A reliable workflow:

1. **Search `soh/include/functions.h` for the old name.** If it's still
   there, your code should compile — check that you've included the right
   header.
2. **If the old name isn't there, grep Shipwright's git history for it:**
   ```sh
   git log -S 'func_80078884' --source --all -- soh/
   ```
   The commit that removed the old name usually introduces the new one in
   the same diff.
3. **Read the CI error carefully.** The compiler's `did you mean?`
   suggestion is sometimes correct but **not always** — see the warning
   below.
4. **Check other Shipwright headers for the type.** Struct field renames
   live in `soh/include/z64save.h`, `soh/include/z64actor.h`, and the
   various `z64*.h` headers. Enum renames live near the enum definition
   (e.g. `PLAYER_IA_*` in `z64player.h`).
5. **When in doubt, grep Shipwright for a nearby, known-good symbol.**
   The surrounding code often reveals the new name.

## Common renames hit in TrifoolPercent

These are the actual translations we needed when porting Triforce% logic
into TrifoolPercent. The list is not exhaustive — more will surface as
new code lands — but it covers the recurring categories.

### Functions

| Decomp / old name | Shipwright name | Notes |
|---|---|---|
| `func_80034A14` | `Npc_TrackPoint` | NPC head/eye tracking toward a point. |
| `func_80078884(sfxId)` | `Sfx_PlaySfxCentered(sfxId)` | **See warning below** — GCC suggests `func_800F8884` for this one, which has a different signature. |
| `Actor_IsTalking` | `Actor_ProcessTalkRequest` | Returns true and advances the talk state in one call. |
| `Actor_RequestToTalk` | `Actor_OfferTalkNearColChkInfoCylinder` | Longer, more descriptive name. |

### Enums

| Decomp / old name | Shipwright name | Notes |
|---|---|---|
| `PLAYER_AP_BOTTLE` | `PLAYER_IA_BOTTLE` | Prefix `PLAYER_AP_` was renamed to `PLAYER_IA_` ("item action") throughout. |
| `SLOT_OCARINA_TIME` | `SLOT_OCARINA` | The Ocarina of Time no longer has its own inventory slot distinct from the Fairy Ocarina — a single slot holds whichever you have. |
| `EVENTCHKINF_OPENED_DOOR_OF_TIME` | `EVENTCHKINF_OPENED_THE_DOOR_OF_TIME` | `_THE_` added. |

### Save-context fields

| Decomp / old name | Shipwright name | Notes |
|---|---|---|
| `gSaveContext.magicAcquired` | `gSaveContext.isMagicAcquired` | `is` prefix added for booleans. |
| `gSaveContext.doubleMagic` | `gSaveContext.isDoubleMagicAcquired` | Same pattern. |

### Room / scene fields

| Decomp / old name | Shipwright name | Notes |
|---|---|---|
| `roomCtx.drawParams[1]` | `roomCtx.unk_74[1]` | Shipwright kept the field as an "unknown" array because its exact semantics vary by scene; decomp named it ahead of Shipwright. |

### Entrance IDs

The decomp uses a positional naming scheme where every entrance in a scene
is numbered: `ENTR_LOST_WOODS_0`, `ENTR_LOST_WOODS_1`, etc. Shipwright
renamed these to descriptive constants:

| Decomp pattern | Shipwright name |
|---|---|
| `ENTR_LOST_WOODS_0` | `ENTR_LOST_WOODS_SOUTH_EXIT` |
| `ENTR_ZORAS_DOMAIN_0` | `ENTR_ZORAS_DOMAIN_ENTRANCE` |
| `ENTR_*_0` (in general) | A descriptive constant per entrance — look it up in `soh/include/entrance_table.h`. |

There is no mechanical rule — you have to look each one up. The
descriptive form is clearer once you're used to it.

## The golden rule

**Shipwright wins. Always rename your ported code to match Shipwright,
never the other way around.**

Do not:

- Add a `#define func_80034A14 Npc_TrackPoint` compatibility macro.
- Add a `typedef` or wrapper function to make the old name resolve.
- Patch a Shipwright header to re-export an old name.

Shipwright is upstream. Your mod is a guest in its build. Compatibility
shims accumulate quickly, obscure which code uses which API, and break the
moment Shipwright renames again. Update your call sites instead.

## Warning: compiler suggestions can be wrong

GCC's `error: 'func_80078884' was not declared in this scope; did you mean
'func_800F8884'?` is tempting to follow blindly. **Don't.** The compiler
guesses by name similarity (edit distance on the identifier string), not
by signature compatibility.

Concrete example from TrifoolPercent:

- Our code called `func_80078884(sfxId)` — a one-argument sound-effect
  trigger.
- Shipwright's `func_800F8884` exists but has signature
  `void func_800F8884(u8 bankId, Vec3f* pos)` — a completely different
  function (positional SFX in a specific bank).
- Following the compiler's suggestion compiles cleanly and silently plays
  the wrong sound (or crashes on the uninitialized `Vec3f*`).

The correct replacement is `Sfx_PlaySfxCentered(sfxId)`, which has the
matching signature.

**Always verify the suggested name's signature in Shipwright's headers
before using it.** `functions.h` lists the full prototype — if the
argument types don't match what you were passing, the suggestion is
wrong even if the names look similar.

## See also

- Shipwright `functions.h`: https://github.com/HarbourMasters/Shipwright/blob/develop/soh/include/functions.h
- Shipwright save-context header: `soh/include/z64save.h`
- Shipwright actor header: `soh/include/z64actor.h`
- Shipwright entrance table: `soh/include/entrance_table.h`
- The N64 OoT decomp (upstream of all the old names):
  https://github.com/zeldaret/oot
