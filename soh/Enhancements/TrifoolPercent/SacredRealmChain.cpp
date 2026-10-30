// TrifoolPercent Sacred Realm chain scaffold.
//
// Phase B of docs/cutscene-plan.md -- the Temple-of-Time -> Chamber-of-Sages ->
// Triforce-Room flow. The scenes and cutscenes are not vendored yet; this file lands
// the dormant scaffold so the hooks are ready to wire the moment Fast64 produces
// scenes/ChamberOfSages/ChamberOfSages_scene.c and the WSRCutscene actor is registered.
//
// Shape (mirrors Ending.cpp):
//   * CVAR_TRIFOOL_SACRED_REALM_CHAIN gates every hook; default off so this file is
//     effectively dead code until the assets land.
//   * OnSceneSpawnActors in SCENE_TEMPLE_OF_TIME: a stub that will spawn the
//     WSRCutscene actor once the song-of-sages trigger fires (today it only reads
//     TrifoolBeat_HasLearnedLongOfTime so the reroute key is in place).
//   * OnPlayerUpdate in the Chamber-of-Sages scene: when the cutscene has ended, grant
//     TrifoolBeat_GrantVisitedChamberOfSages and reroute to the Triforce Room entrance
//     (replaces vanilla terminator 82's fade-to-black). Same shape as
//     LeaveTriforceRoomAtCutsceneEnd / LeaveEndingAtCutsceneEnd.
//
// Scene / entrance id notes:
//   * Chamber of the Sages (kenjyanoma): vanilla scene id 0x43. Not referenced by any
//     SCENE_* constant currently in the TrifoolPercent sources, so this file uses the
//     numeric id with a sanity comment -- same tactic Setup.cpp's dev warps use for
//     entrance ids (0x006B etc.). When SoH-side SCENE_CHAMBER_OF_THE_SAGES becomes
//     available in the codebase, swap the numeric for the symbol.
//   * Triforce Room entrance: TrifoolPercent.cpp already uses ENTR_BESITU_0 via
//     SCENE_BESITU; the entrance id used by the dev warp is 0x060C (see Setup.cpp's
//     warp list where Besitu is reached by ENTR_SYOTES_0 and the sub-slot off that).
//     Here we reuse ENTR_BESITU_0 by name so the chain lands at the Triforce Room's
//     canonical entry (header00 cutscene fires), not a secondary spawn.

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"
#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

extern "C" {
#include "TrifoolPercent.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define CVAR_SACRED_REALM_CHAIN_VALUE CVarGetInteger(CVAR_TRIFOOL_SACRED_REALM_CHAIN, 0)

// Vanilla OoT scene id for kenjyanoma (Chamber of the Sages). Not yet aliased to a
// SCENE_* constant in the files vendored under soh/Enhancements/TrifoolPercent/.
static constexpr s32 kSceneChamberOfSages = 0x43;

// Rough frame budget before the Chamber-of-Sages cutscene would hand off to the next
// scene. Pulled from the plan's §3.3 note that the cutscene is the chamber's main
// payload; actual value gets re-tuned once the Fast64 export of ChamberOfSages.blend
// is in the repo and we can read CS_END from the generated cutscene data.
#define SACRED_REALM_CS_EXIT_FRAME 300

static bool IsChamberOfSages() {
    return gPlayState != nullptr && gPlayState->sceneNum == kSceneChamberOfSages;
}

static bool IsTempleOfTime() {
    return gPlayState != nullptr && gPlayState->sceneNum == SCENE_TEMPLE_OF_TIME;
}

// Phase B §B.3: WSRCutscene actor spawn hook. Dormant until the actor file lands.
// Reads the Long-of-Time flag as the trigger key -- upstream fires on OnOcarinaSongAction
// for the custom song, which calls TrifoolBeat_GrantLearnedLongOfTime first.
static void TriggerWarpToSacredRealm() {
    if (!IsTempleOfTime()) {
        return;
    }
    if (!TrifoolBeat_HasLearnedLongOfTime()) {
        return;
    }
    // Scaffold-only: real spawn calls Actor_Spawn for the WSRCutscene actor once it is
    // registered in ActorDB. Leaving this empty is intentional -- firing cutsceneTrigger
    // here without the actor's segment set would crash Cutscene_SetSegment.
}

// Phase B §B.6: Chamber-of-Sages exit reroute. When the chamber's cutscene has run its
// course we warp Link to the Triforce Room entrance -- the vanilla terminator case 82
// would have faded to kokiri at this point, which Triforce% replaced wholesale.
static void LeaveChamberOfSagesAtCutsceneEnd() {
    if (!IsChamberOfSages()) {
        return;
    }
    CutsceneContext* csCtx = &gPlayState->csCtx;
    if (csCtx->state == CS_STATE_IDLE || csCtx->frames < SACRED_REALM_CS_EXIT_FRAME ||
        gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    TrifoolBeat_GrantVisitedChamberOfSages();
    gPlayState->nextEntranceIndex = ENTR_BESITU_0;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
}

static void RegisterTrifoolSacredRealmChain() {
    COND_HOOK(OnSceneSpawnActors, CVAR_SACRED_REALM_CHAIN_VALUE, TriggerWarpToSacredRealm);
    COND_HOOK(OnPlayerUpdate, CVAR_SACRED_REALM_CHAIN_VALUE, LeaveChamberOfSagesAtCutsceneEnd);
}

static RegisterShipInitFunc sacredRealmChainInitFunc(RegisterTrifoolSacredRealmChain,
                                                    { CVAR_TRIFOOL_SACRED_REALM_CHAIN });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterSacredRealmChainWidget() {
    WidgetPath path = { "TrifoolPercent", "Cutscenes", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Sacred Realm chain (scaffold)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_SACRED_REALM_CHAIN)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Phase B scaffold: Temple-of-Time -> Chamber-of-Sages -> Triforce-Room reroute. "
            "Dormant until the Chamber-of-Sages scene (ChamberOfSages.blend) and WSRCutscene "
            "actor land. Toggling this on today just arms the Chamber-exit -> Triforce-Room "
            "warp; vanilla kenjyanoma visits will fade to the Triforce Room after "
            "~300 cutscene frames once this is on."));
}

