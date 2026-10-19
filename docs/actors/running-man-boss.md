# Running Man boss (`BossRunningMan`)

The Triforce% Running Man boss ships three different Running Man states, two
of which TrifoolPercent still owes:

1. **Running Man race** (`actor/z_en_mm3`, rewritten upstream). Bunny-Hood +
   time-travel lets Link finish before the race starts ("How did you get here
   before I did?!"). Not covered here.
2. **Running Man boss fight** (`actor/BossRunningMan`, 1,679 lines upstream).
   Four-ghost IK-rigged skeleton, run / dodge / kick / arrow states, intro
   cutscene. **Not yet ported.**
3. **Post-fight NPC form** (same actor, `npc` sub-union). Hands over the
   Sages' Charm through five textboxes (0x0C30..0x0C34), then runs off.
   **Dialogue + handoff scaffolded** in `soh/Enhancements/TrifoolPercent/BossRunningMan.cpp`.

## What landed in the scaffold slice

| Thing | Where | Status |
|---|---|---|
| Dialogue (0x0C30..0x0C34) | `BossRunningMan.cpp` `kRunningManC30..C34` | ✅ copied verbatim from upstream `assets/text/messages.c` via CustomMessage |
| Sages' Charm handoff on 0x0C34 close | `BossRunningMan.cpp` `TickRunningManHandoff` | ✅ grants charm + fires `TRIFOOL_TEXT_SAGES_CHARM_GET` |
| CVar + debug pulse | `CVAR_TRIFOOL_RUNNING_MAN` + `.RunningManDialoguePulse` | ✅ menu widgets |

Testing the scaffold: enable the Running Man beat + the Beat 2.0 Sages' Charm
beat, then tick "Start Running Man dialogue (debug)". The five regret
textboxes play; on close the Sages' Charm item-get card renders and
`TrifoolBeat_HasSagesCharm()` reads true.

## Follow-up slices

Each row is its own future PR.

### 1. Vanilla En_Mm ride-on scene glue

Spawn vanilla `ACTOR_EN_MM` at a known Hyrule Field position when
`CVAR_TRIFOOL_RUNNING_MAN` is on and the player has not yet received the
Charm. Intercept Link's talk-to action with a GameInteractor hook that
replaces the vanilla greeting with `TRIFOOL_TEXT_RUNNING_MAN_C30`, so the
dialogue scaffold is reachable without the debug pulse. This buys us a
visible, animated placeholder (the vanilla OoT Running Man already uses
`OBJECT_MM`) while the custom model is blocked on Blender.

### 2. Custom model port

Upstream `actors/BossRunningMan/BossRunningManObj.h/.ld` declare the symbol
table but the Fast64 ObjEx subfiles (`BossRunningManObj.anim`, `.mtlex`,
`.skel`) have not been exported. Needs Isaac's Blender machine to produce
the `.c` + `.pat` the Torch pipeline consumes, then a `TrifoolPercent.o2r`
build that includes the new object. Same shape as the Sages' Charm
`object_gi_sagescharm.o2r` follow-up.

Once the object lands, the scaffold swaps the vanilla En_Mm ride-on for a
custom `TfBossRunningMan` actor file that renders the ported skeleton. The
actor file does not need to exist until the model is available -- a C file
that references symbols from a missing object does not link.

### 3. BetaGiantRupee drop

Upstream `RunningMan_Init` spawns `ACTOR_EN_EX_RUPPY` with param
`ACTORPARAM_BETAGIANTRUPEE` (0x0001) at the boss's spawn position; the
rupee explodes when picked up. Needs the `BetaGiantRupee` actor port
(not yet in the repo) and its own object.

### 4. Boss AI port

The upstream state machine (`actor/BossRunningMan/BossRunningMan.c`), in
rough order of dependency:

| Upstream function | Role | SoH port notes |
|---|---|---|
| `RunningMan_Init` | spawn + BetaGiantRupee + flag gate | gated on #3 + the custom save-manager pass |
| `RunningMan_ChangeToBoss` | collider + anim swap into fight | needs the four-ghost after-image system |
| `RunningMan_UpdateIntro` | poll BetaGiantRupee pickup + trigger `BossRunningManIntroCS` | needs cutscene data ported (`BossRunningManCutscenes.c`, 75 lines) |
| `RunningMan_Run` / `_Dodge` / `_Kick` / `_Arrow*` | combat states | pure state-machine port, each is its own slice |
| `RunningMan_Hurt` / `_HurtLand` | damage reaction + knockback | needs the collider matrix set up |
| `RunningMan_SetupOutro` / `_Outro` | death + exploding giant rupee | depends on #3 |
| `RunningMan_ChangeToNPC` | post-fight transition | the state the scaffold's dialogue currently stands in for |
| `RunningMan_UpdateRunAway` | run off after charm handoff | needs the custom `AnimRunFast` animation |

### 5. Magic bar 2x scale-up

Upstream `RunningMan_UpdateDialogue` at `npc.timer == 25` does
`gSaveContext.unk_13F0 = 9` (magic mode = "filling"), halves current magic,
and raises `unk_13F6` to 0x60 so the engine redraws the bar at double
width. Visual-only. Overlaps with `docs/items/sages-charm.md` follow-up 3
(magic bar visual tweak), so lands together.

### 6. Intro cutscene

`BossRunningManIntroCS` is a 75-line cutscene in
`actors/BossRunningMan/BossRunningManCutscenes.c` -- a reasonable size for a
direct Triforce% `.c` → SoH `CutsceneData` conversion, same pipeline that
built `gTrifoolTriforceRoomCs` and `gTrifoolTestRoomPrototypeCs`.

## Upstream references

- `actors/BossRunningMan/BossRunningMan.c` -- the full 1,679-line state machine.
- `actors/BossRunningMan/BossRunningMan.h` -- struct layout (`RunManState` bitfield,
  `boss` / `npc` sub-union, `ColliderSet`).
- `actors/BossRunningMan/BossRunningManObj.h` -- object symbol table (texture,
  material, DL, skeleton, animation names). ObjEx sources missing.
- `actors/BossRunningMan/BossRunningManCutscenes.c` -- intro cutscene data.
- `assets/text/messages.c` ids `0x0C30..0x0C34` and `0x0068` -- NPC dialogue
  and Sages' Charm item-get card.
