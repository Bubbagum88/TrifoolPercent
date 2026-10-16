# Sourcing the Beta Great Fairy zobj

**Status, 2026-10-16:** Dead end under the project's content policy. The actor ships
without a mesh; `TfBetaGreatFairy_Draw` is a no-op and only the particles and audio
cues render.

## The policy

Beta assets in TrifoolPercent come only from retail OoT via Torch/OTRExporter against
a legal OoT ROM. External fan-made beta asset packs are not permitted.

## Why the beta fairy fails that policy

Triforce%'s `actor/BetaGreatFairy/betagreatfairy.c:123,132` draws two display lists:

- `DL_BETAGREATFAIRY = 0x0600D1B0`
- `DL_BETAGREATFAIRY_FLOWER = 0x0600DC68`

These are offsets into segment 6, which Triforce% loaded with a custom object (their
`#define OBJ_ID 0x00A` slot had the beta mesh packed in). That custom zobj was
**never checked into the Triforce% repo** — the one input `actor/BetaGreatFairy/`
needs that isn't shipped. The "Beta" in the actor's name is literal: the mesh is a
cut-content model of the Great Fairy that **isn't in retail OoT either**. OoT's
retail Great Fairy objects (`object_oF1d_map` and friends) hold the adult/child
final-release fairy, not the beta pose/model those display lists reference.

So the two sources we're allowed to use — Triforce% itself and retail OoT via
Torch — don't have the asset. The only way to get it would be a fan-made beta pack
(ruled out) or a from-scratch recreation (its own project).

## What ships instead

`TfBetaGreatFairy_Draw` returns without drawing. The actor's state machine still
runs: it plays `NA_SE_EV_GREAT_FAIRY_APPEAR/VANISH` and `NA_SE_VO_FR_LAUGH_0`,
spawns `EffectSsKiraKira_SpawnFocused` sparkles, and advances through
hidden → rising → floating → falling states. In-game, the player sees sparkles
and hears her, but her mesh isn't there.

This is a known gap, not a bug to chase. The actor code is in place for when the
mesh problem is solved by another route (e.g. the project explicitly adds a
from-scratch model, or the policy later admits a specific beta pack).

## Key references

- `third_party/triforce-percent/actor/BetaGreatFairy/betagreatfairy.c:123,132` — the
  two display-list calls.
- `third_party/triforce-percent/actor/BetaGreatFairy/obj.h` — the two DL addresses,
  no zobj alongside.
- `soh/Enhancements/TrifoolPercent/actors/z_tf_beta_great_fairy.c` — our port, with
  the empty draw.
- `docs/roadmap.md` Phase 4 — the fairy actor is marked ported with "mesh
  unavailable until the zobj is sourced"; this doc is why.
