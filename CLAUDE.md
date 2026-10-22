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

## C++ type safety

MSVC promotes **C4309** (truncation of constant value) to an **error**.

- Use **explicit casts** (`static_cast<T>(...)`) for every narrowing conversion.
- **No implicit truncation of constants** — write the cast or widen the type.

## Use the engine, don't reinvent it

When vanilla OoT / SoH already provides a behavior — hover, idle anims, SFX, actor flags, cutscene commands — **use it**.

- Only write custom code for what the engine genuinely doesn't have.
- Prefer engine-native actor behaviors over hand-rolled equivalents.
- If you're about to re-implement something that sounds like it should already exist, grep SoH first.

## Quick checklist before you code

- [ ] Listed every file I'll touch (≤ 5, or proposed a split)
- [ ] Explained goal + approach in plain language and got Isaac's go
- [ ] Looked up the real Shipwright symbol names
- [ ] Confirmed no existing engine behavior does this already
- [ ] No new dependency being added without approval
- [ ] Explicit `static_cast<>` on every narrowing conversion
- [ ] Have a 3-step in-game test ready for Isaac
