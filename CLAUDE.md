# CLAUDE.md — Coding Guidelines for AI Agents

Guidelines for every AI agent working in this repository. Read before touching code.

## Project context

TrifoolPercent is a C++ mod on **Ship of Harkinian (SoH / Shipwright)** that reimagines the Triforce% ACE/TAS run as a native SoH experience — custom scenes, actors, cutscenes, and logic running cleanly inside SoH.

**Tech stack**
- C++ mod on SoH/Shipwright
- MSVC on Windows CI (strict — C4309 is an error)
- Linux CI for `.o2r` asset build
- **No new libraries, frameworks, or services without Isaac's approval**

## Working with Isaac

Isaac drives this project. He is a **non-coder** — he won't read source. Every change needs a plain-language explanation and a way to verify it in-game.

### Before touching files
- **List every file you plan to modify first.** If the list is **> 5**, stop and propose splitting the task.
- **Before writing code for any non-trivial change**, explain in plain language:
  - The goal
  - The planned approach
- **Wait for Isaac's go** before coding.

### After any change
- Provide a **3-step manual test** Isaac can run in SoH / in the game to verify the change.
- Steps must be concrete clicks / inputs / observations — not "check that X works."

### When fixes fail
If **three consecutive fix attempts fail**, STOP. Propose:
- **(a)** revert
- **(b)** what we know vs. what we don't know
- **(c)** a different approach

### Core experience is sacred
If a proposed change would make the core mod experience worse — Triforce quest, beats, song engine, cutscenes — **slower, less reliable, or more confusing** — stop and flag it, **even if asked**.

## Root cause, not band-aids

> **NEVER implement workarounds or band-aid solutions — ALWAYS fix the root cause.**

- No `try/catch` to swallow a bug you don't understand.
- No "if (crashed) return;" guards around the symptom.
- No silently skipping the broken path.
- If you can't find the root cause, say so — don't paper over it.

## Shipwright wins on naming

Shipwright is the source of truth for symbol names.

- **Rename TrifoolPercent code to match Shipwright symbols.** Never the other way around.
- **Never** rename a Shipwright symbol.
- **Never** add `typedef` / `#define` aliases or compat shims to bridge a naming mismatch.
- **Before writing code, verify the symbol name in Shipwright headers.** The pinned SHA is in `tools/setup.sh` as `SHIPWRIGHT_COMMIT`.

### Known renames we've been bitten by

Treat anything in this list as a tripwire — if your code uses the left-hand name, it's wrong:

- `Actor_IsTalking` → `Actor_ProcessTalkRequest`
- `roomCtx.drawParams` → `roomCtx.unk_74`
- `SLOT_OCARINA_TIME` → `SLOT_OCARINA`
- `gSaveContext.magicAcquired` → `isMagicAcquired`
- `gSaveContext.doubleMagic` → `isDoubleMagicAcquired`
- `ENTR_LOST_WOODS_0` → `ENTR_LOST_WOODS_SOUTH_EXIT`
- `ENTR_ZORAS_DOMAIN_0` → `ENTR_ZORAS_DOMAIN_ENTRANCE`
- `SCENE_SHOP1` → `SCENE_BAZAAR`
- `MATRIX_NEW` (renamed; grep Shipwright for current spelling)
- Any N64-decomp `func_80xxxxxx` name — SoH exposes these under human-readable names (`Actor_*`, `Play_*`, `Message_*`). See `docs/porting-guide/`.

### Know which struct a field lives on

`cutsceneTrigger` is on `PlayState` (write `gPlayState->cutsceneTrigger`), **not** `SaveContext`. When in doubt, open the header and read the struct — don't guess from surrounding code.

### Never invent enumerator names

If you can't find `EVENTCHKINF_UNFROZE_KING_ZORA` in `z64save.h`, it doesn't exist. Use the numbered form (`EVENTCHKINF_4B`) or wrap the real symbol in a local `#define`. Hallucinated names are our most common CI break.

### Bare clones can't see Shipwright symbols

Anything under `third_party/Shipwright/soh/include/` exists only after `tools/setup.sh`. Never claim a `gSaveContext.*` field, `ENTR_*`, `OBJECT_*`, `GID_*`, `COND_HOOK`, or `GameInteractor::On*` signature is broken from a bare clone — run the syntax-check CI or populate the submodule first. From a bare clone you can only safely flag: locally-defined `CVAR_TRIFOOL_*`, in-repo type mismatches, and hallucinated names used only in comments.

## C++ type safety

MSVC promotes **C4309** (truncation of constant value) to an **error**.

- **Explicit `static_cast<T>(...)` on every narrowing conversion.** No implicit truncation of constants — write the cast or widen the type.
- **Any constant at or near a signed/unsigned boundary gets an explicit `static_cast<>`.** `constexpr s16 x = 0x8000` is a C4309 error; write `static_cast<s16>(0x8000)` (or `-0x8000`). Applies to any hex rotation/angle and any top-bit flag.
- **Never pass a raw `u8` / `s16` / integer literal to an `enum class` parameter.** `CustomMessage(..., TextBoxType, TextBoxPosition, ...)`, `GI_*`, `ItemID`, `PLAYER_IA_*`, `PLAYER_MASK_*`, `TRANS_TYPE_*`, `TEXTBOX_ICON_*` — pass the typed enumerator, or `static_cast` at the call site. Clang `-Werror=enum-conversion` catches it.
- **Every `switch` on an `enum class` needs a `default:`.** `-Wswitch` is fatal. If you handle every case, still add `default: break;`.
- **Fix sign-compare at the cast, not the comparison.** Prefer `static_cast<size_t>(i) < v.size()`.

## Linker / symbol wiring

- **Every `extern` declaration needs a matching definition** in a .cpp compiled into the same target. Grep for the definition before writing the extern; grep again before committing. Missing definitions surface at link time, not compile time, so static review has to catch them.
- **Every `CVAR_TRIFOOL_*` used in a .cpp must be `#define`d in `TrifoolPercent.h`** (or defined locally with a comment). A missing CVAR macro surfaces as a cryptic preprocessor error on the `RegisterShipInitFunc` line, not where you used it.

## `extern "C"` and header hygiene

- **Only wrap pure-C Shipwright headers in `extern "C"`.** C-side headers (`macros.h`, `functions.h`, `variables.h`, `z64save.h`, `z64cutscene_commands.h`, actor `z_en_*.h`) go inside. TrifoolPercent-side headers (`TrifoolPercent.h`, `BeatFlags.h`, any C++ header) stay outside — wrapping them is a compile error.
- **Don't include SoH's own enhancement headers (`soh/Enhancements/...`) inside `extern "C"`.** They're C++.
- **If a file uses `GET_EVENTCHKINF` / `SET_EVENTCHKINF` or touches `gSaveContext`, include `z64save.h` explicitly** rather than relying on the transitive include through `variables.h`.

## Use the engine, don't reinvent it

When vanilla OoT / SoH already provides a behavior — hover, idle anims, SFX, actor flags, cutscene commands — **use it**.

- Only write custom code for what the engine genuinely doesn't have.
- Prefer engine-native actor behaviors over hand-rolled equivalents.
- If you're about to re-implement something that sounds like it should already exist, grep SoH first.

## Patterns that look wrong but aren't

Don't "fix" these — they've been investigated:

- **`RegisterShipInitFunc(name, {CVAR_FOO}, ...)` brace-init** — builds a `std::set<std::string>` from a string literal. Fine.
- **Debug-scene entrance IDs** (`ENTR_SYOTES_0`, `ENTR_TEST01_0`, `ENTR_BESITU_0`) use the `_0` suffix and are real. Only shipping scenes use the descriptive-suffix form.
- **`GameInteractor` hook signatures drift** between Shipwright versions — before adding a `RegisterGameHook<...>`, copy the template arg + lambda signature from the current `GameInteractorHookTable.h`. Don't retype from memory.

## CI workflow

- **Prefer the syntax-check CI job for a fast signal** — it compiles TrifoolPercent against stubbed deps in ~1 minute; full `build-soh` takes much longer. If syntax-check is green, most of the technical rules above are satisfied.
- **When CI is red, read the actual error** — don't speculate. CI's symbol errors always name the real symbol it couldn't resolve, which is usually one grep away from the right name.
- Branch from `origin/main`. Push to the branch the thread system assigns; never force-push to `main`. One PR per logical change.

## Quick checklist before you code

- [ ] Listed every file I'll touch (≤ 5, or proposed a split)
- [ ] Explained goal + approach in plain language and got Isaac's go
- [ ] Looked up the real Shipwright symbol names (checked `SHIPWRIGHT_COMMIT`)
- [ ] Confirmed no existing engine behavior does this already
- [ ] No new dependency being added without approval
- [ ] Explicit `static_cast<>` on every narrowing conversion
- [ ] Every new `extern` and `CVAR_TRIFOOL_*` has a matching definition
- [ ] Have a 3-step in-game test ready for Isaac
