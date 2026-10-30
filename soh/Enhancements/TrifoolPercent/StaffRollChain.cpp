// TrifoolPercent Staff Roll chain scaffold.
//
// Phase G of docs/cutscene-plan.md -- the ten vanilla-scene flyovers chained one into
// the next, culminating in StaffRollFinaleCS. Order (plan §2b):
//
//   Ending -> spot00 (Hyrule Field) -> spot16 (DMT) -> spot09 (Gerudo Valley) ->
//   ydan (Deku Tree) -> spot08 (Zora's Fountain) -> spot03 (Zora's River) ->
//   spot04 (Kokiri Forest) -> spot20 (Lon Lon Ranch) -> spot13 (Haunted Wasteland) ->
//   spot15 (Hyrule Castle) -> back to title.
//
// Each flyover is a camera + text + BGM cutscene attached to a vanilla scene header;
// Triforce% chained them by patching the terminator switch (case ids 84-93). SoH does
// not patch that switch -- we watch csCtx.frames in a scene-id-keyed OnPlayerUpdate
// handler and set nextEntranceIndex + transitionTrigger ourselves (see plan §1
// "CS_TERMINATOR destinations").
//
// This file lands the dormant chain table and the handler shape so the moment the
// flyover C sources (scenes/spot00/HyruleFieldFlyover.c, scenes/spot16/..., etc.) are
// wired into SoH's resource table, flipping CVAR_TRIFOOL_STAFF_ROLL_CHAIN on routes
// them in order. The vanilla scenes are populated by the base game, so toggling this
// on today will reroute an on-foot visit to any of these scenes once an in-scene
// cutscene (any cutscene) crosses the per-step frame budget -- which is why the
// default is off and the frame budget sits at a conservatively high placeholder.
//
// Scene / entrance id notes:
//   * Scene ids below use SCENE_HYRULE_FIELD etc. which already exist in SoH's enum
//     (the same enum SCENE_KOKIRI_FOREST and SCENE_LOST_WOODS -- used elsewhere in
//     TrifoolPercent -- come from). Entrance ids are the vanilla "entry 0" of each
//     scene; the plan's §3.10 note says the flyover's scene-header cutscene is
//     reached through the entrance's cutsceneIndex, which the engine already sets
//     when gEntranceCutsceneTable carries an entry for that scene's base entrance.
//   * StaffRollFinaleCS (the fade-to-title at the end of the chain) lives in
//     Ending_scene.c; the final step in the chain hands back to the Ending slot and
//     relies on EndingLeaveAtCutsceneEnd (see Ending.cpp) to fade to Kokiri Forest.
//     Once a dedicated title-return exists we rewire that.

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

#define CVAR_STAFF_ROLL_CHAIN_VALUE CVarGetInteger(CVAR_TRIFOOL_STAFF_ROLL_CHAIN, 0)

// Vanilla OoT scene ids for the flyover chain. Several of these are not yet referenced
// by name in soh/Enhancements/TrifoolPercent/*.cpp; using the numeric literal here
// keeps the file compilable regardless of which SCENE_* aliases SoH ships.
// Reference: Triforce% statics/scene.c switch cases 84-93.
//   spot00 = Hyrule Field          (vanilla scene 0x51)
//   spot16 = Death Mountain Trail  (vanilla scene 0x60)
//   spot09 = Gerudo Valley         (vanilla scene 0x5A)
//   ydan   = Deku Tree             (vanilla scene 0x00)
//   spot08 = Zora's Fountain       (vanilla scene 0x59)
//   spot03 = Zora's River          (vanilla scene 0x54)
//   spot04 = Kokiri Forest         (vanilla scene 0x55)  (SCENE_KOKIRI_FOREST)
//   spot20 = Lon Lon Ranch         (vanilla scene 0x63)
//   spot13 = Haunted Wasteland     (vanilla scene 0x5D)
//   spot15 = Hyrule Castle         (vanilla scene 0x5F)

struct StaffRollLink {
    s32 sceneId;      // scene we're in and watching for cutscene end
    s32 nextEntrance; // vanilla "entry 0" of the next scene in the chain
    const char* label;
};

// Entrance ids below are the vanilla ENTR_<SCENE>_0 slots. The plan's §3.10 notes that
// gEntranceCutsceneTable already carries the needed entries for 84-93 in vanilla.
// Entrance ids are vanilla OoT "entry 0" values (well-known constants matched by SoH's
// ENTR_* enum). Kept numeric here so the file compiles cleanly regardless of which
// ENTR_* aliases happen to be in scope -- a follow-up slice swaps each for its ENTR_*
// name once the chain is wire-tested.
static const StaffRollLink kStaffRollChain[] = {
    { 0x51,                0x04FD,         "spot00 -> spot16 (DMT)"             },
    { 0x60,                0x0129,         "spot16 -> spot09 (Gerudo Valley)"   },
    { 0x5A,                0x0000,         "spot09 -> ydan (Deku Tree)"         },
    { 0x00,                0x0088,         "ydan -> spot08 (Zora's Fountain)"   },
    { 0x59,                0x00EA,         "spot08 -> spot03 (Zora's River)"    },
    { 0x54,                0x00EE,         "spot03 -> spot04 (Kokiri Forest)"   },
    { SCENE_KOKIRI_FOREST, 0x0157,         "spot04 -> spot20 (Lon Lon Ranch)"   },
    { 0x63,                0x0130,         "spot20 -> spot13 (Haunted Waste)"   },
    { 0x5D,                0x0138,         "spot13 -> spot15 (Hyrule Castle)"   },
    // Last link: spot15 -> back to the Ending scene for StaffRollFinaleCS.
    // ENTR_SYOTES2_0 is the Triforce% Ending slot (see Ending.cpp); kept symbolic
    // because that symbol is already exercised by Ending.cpp.
    { 0x5F,                ENTR_SYOTES2_0, "spot15 -> Ending"                   },
};

static constexpr size_t kStaffRollChainCount = sizeof(kStaffRollChain) / sizeof(kStaffRollChain[0]);

// Conservative frame budget before the chain reroute arms. The flyovers themselves run
// 150-400 frames each (plan §3.10); until the C sources are vendored and we can read
// each CS_END, this placeholder keeps the handler from firing on short vanilla
// cutscenes (e.g. the Kokiri Forest Deku Tree intro, which is why the default is off).
#define STAFF_ROLL_LINK_EXIT_FRAME 450

static const StaffRollLink* FindCurrentLink() {
    if (gPlayState == nullptr) {
        return nullptr;
    }
    const s32 sceneId = gPlayState->sceneNum;
    for (size_t i = 0; i < kStaffRollChainCount; ++i) {
        if (kStaffRollChain[i].sceneId == sceneId) {
            return &kStaffRollChain[i];
        }
    }
    return nullptr;
}

static void AdvanceStaffRollChain() {
    const StaffRollLink* link = FindCurrentLink();
    if (link == nullptr) {
        return;
    }
    CutsceneContext* csCtx = &gPlayState->csCtx;
    if (csCtx->state == CS_STATE_IDLE || csCtx->frames < STAFF_ROLL_LINK_EXIT_FRAME ||
        gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    gPlayState->nextEntranceIndex = link->nextEntrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
}

static void RegisterTrifoolStaffRollChain() {
    COND_HOOK(OnPlayerUpdate, CVAR_STAFF_ROLL_CHAIN_VALUE, AdvanceStaffRollChain);
}

static RegisterShipInitFunc staffRollChainInitFunc(RegisterTrifoolStaffRollChain,
                                                  { CVAR_TRIFOOL_STAFF_ROLL_CHAIN });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterStaffRollChainWidget() {
    WidgetPath path = { "TrifoolPercent", "Cutscenes", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Staff Roll chain (scaffold)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_STAFF_ROLL_CHAIN)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Phase G scaffold: chains the ten vanilla-scene flyovers (Hyrule Field through "
            "Hyrule Castle) and then returns to the Ending slot for StaffRollFinaleCS. "
            "Dormant until the flyover C sources under scenes/spot*/ are compiled into SoH. "
            "Toggling on today will reroute scene exits once any in-scene cutscene crosses "
            "~450 frames -- off by default so vanilla visits are unaffected."));
}

