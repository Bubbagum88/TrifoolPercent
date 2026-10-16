// TrifoolPercent Ending scene glue.
//
// Vanilla Triforce% ends with a dedicated "Ending" scene that holds the Sacred Realm
// gazebo + goddess terminator + staff roll cutscenes. The scene source lives in
// scenes/Ending/ (ZAPD-style Ending_scene.c + Ending_scene.h, 735 LOC) and gets exported
// through Fast64 by scenes/ending/export_ending.py into the SCENE_SYOTES2 (0x69) slot --
// the next unused debug scene after SCENE_SYOTES (hex hub) and SCENE_BESITU (Triforce
// Room). SoH loads the exported resources from TrifoolPercent.o2r at the usual path.
//
// This file supplies the dynamic half: scene detection, the entrance cutscene trigger
// (header00's FinaleCS), and a fade back to Kokiri Forest when the cutscene ends -- the
// same shape LeaveTriforceRoomAtCutsceneEnd uses in TrifoolPercent.cpp. Until a proper
// StaffRollFinaleCS wire-up lands, every visit replays FinaleCS and fades out.
//
// The scene is opt-in behind CVAR_TRIFOOL_ENDING (default off). Nothing here can run
// before scenes/ending/export_ending.py has produced the SCENE_SYOTES2 resources; the
// hooks no-op when the player is not in the slot.

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
}

#define CVAR_ENDING_VALUE CVarGetInteger(CVAR_TRIFOOL_ENDING, 0)
#define ENDING_SCENE SCENE_SYOTES2
#define ENDING_ENTRANCE ENTR_SYOTES2_0
#define ENDING_EXIT ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE

// Rough frame count the Triforce% Ending cutscene (FinaleCS) runs for before upstream
// would have handed off to the staff-roll scene. Lifted from the scene's own CS_END frame
// in Ending_scene.c (`CS_BEGIN_CUTSCENE(..., 500)` on header00); re-tune in follow-up once
// the real staff-roll chain lands.
#define ENDING_CS_EXIT_FRAME 500

static bool IsEndingScene() {
    return gPlayState != nullptr && gPlayState->sceneNum == ENDING_SCENE;
}

static void EnterEndingScene() {
    if (!IsEndingScene()) {
        return;
    }
    // The scene's own header00 declares FinaleCS as its cutscene data. Trigger it on
    // every entry so the player always sees the cutscene from the start.
    gSaveContext.cutsceneTrigger = 1;
}

static void LeaveEndingAtCutsceneEnd() {
    if (!IsEndingScene()) {
        return;
    }
    CutsceneContext* csCtx = &gPlayState->csCtx;
    if (csCtx->state == CS_STATE_IDLE || csCtx->frames < ENDING_CS_EXIT_FRAME ||
        gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    gPlayState->nextEntranceIndex = ENDING_EXIT;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
}

static void RegisterTrifoolEnding() {
    COND_HOOK(OnSceneSpawnActors, CVAR_ENDING_VALUE, EnterEndingScene);
    COND_HOOK(OnPlayerUpdate, CVAR_ENDING_VALUE, LeaveEndingAtCutsceneEnd);
}

static RegisterShipInitFunc endingInitFunc(RegisterTrifoolEnding, { CVAR_TRIFOOL_ENDING });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolEndingWidgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Ending scene", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_ENDING)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "The Triforce% Ending (Sacred Realm gazebo + Finale cutscene), in the unused SCENE_SYOTES2 "
            "slot. Needs TrifoolPercent.o2r built with scenes/ending/export_ending.py; without it the "
            "slot stays empty. Every visit replays FinaleCS and fades back to Kokiri Forest. "
            "StaffRollFinaleCS and the song-of-storms-to-staff-roll handoff come later."));
}

static RegisterMenuInitFunc endingMenuInitFunc(RegisterTrifoolEndingWidgets);
