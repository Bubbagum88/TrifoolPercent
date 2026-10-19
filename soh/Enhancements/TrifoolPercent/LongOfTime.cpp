// TrifoolPercent -- Long of Time age-flip consumer.
//
// SongEngine.cpp runs the chromatic matcher and parks a Long of Time match in
// sLastMatchedSong when Link finishes the eight-note Long of Time sequence.
// This module is the consumer: it polls the engine on each frame, consumes a
// Long of Time match, and reloads the current scene with Link's age flipped.
//
// Upstream reference: Triforce% scripts/statics/statics.c Statics_TimeTravel.
// Upstream flips globalCtx->linkAgeOnLoad to the opposite of the saved age,
// kicks a scene reload at gSaveContext.entranceIndex, and uses a quick fade
// transition. We mirror the shape with SoH's native transition fields instead
// of the raw N64 respawn / sceneLoadFlag plumbing:
//
//   gPlayState->linkAgeOnLoad = gSaveContext.linkAge ^ 1;
//   gPlayState->nextEntranceIndex = gSaveContext.entranceIndex;
//   gPlayState->transitionTrigger = TRANS_TRIGGER_START;
//   gPlayState->transitionType    = TRANS_TYPE_FADE_BLACK_FAST;
//
// Upstream also skips the flip in the Temple of Time (vanilla pedestal scene,
// which already owns the age swap) and in the Gerudo Training Ground (upstream
// used the slot for its showcase; TrifoolPercent inherits the exclusion for
// safety). We skip the same two.
//
// This runs alongside the SongEngine detection toggle; the engine must be on
// for a match to land in the first place.
//
// Adapted from Triforce% scripts/statics/statics.c Statics_TimeTravel
// (c) 2019-2022 The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt.

#include <cstdint>

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

// Exported from SongEngine.cpp. Mirrors the TrifoolSongId enum there; keep
// the single magic value in sync with the enum (TRIFOOL_SONG_LONG_OF_TIME = 1).
uint8_t TrifoolSongEngine_PeekLastMatchedSong(void);
uint8_t TrifoolSongEngine_ConsumeMatch(void);
}

#define CVAR_AGE_FLIP_VALUE CVarGetInteger(CVAR_TRIFOOL_LONG_OF_TIME_AGE_FLIP, 0)

// Must match TrifoolSongId::TRIFOOL_SONG_LONG_OF_TIME in SongEngine.cpp.
static constexpr uint8_t kTrifoolSongLongOfTime = 1;

// Scenes where upstream Triforce% refuses to time-travel. Temple of Time is
// vanilla's own pedestal swap; Gerudo Training Ground was repurposed upstream
// and kept out of the time-travel path. Guarding here means a stray match in
// those scenes is swallowed without a scene reload.
static bool IsBlockedScene() {
    if (gPlayState == nullptr) return true;
    switch (gPlayState->sceneNum) {
        case SCENE_TEMPLE_OF_TIME:
        case SCENE_GERUDO_TRAINING_GROUND:
            return true;
        default:
            return false;
    }
}

// Per-frame polling consumer. OnPlayerUpdate is called every frame the player
// actor is live, which is also when the ocarina interface runs, so a match
// lands on the same frame Link finishes the eighth note.
static void TickLongOfTime() {
    if (!CVAR_AGE_FLIP_VALUE) return;
    if (gPlayState == nullptr) return;
    // Already transitioning (e.g. a vanilla warp song fired on the same frame).
    // Leave the match parked so the next frame can consume it cleanly.
    if (gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) return;
    if (TrifoolSongEngine_PeekLastMatchedSong() != kTrifoolSongLongOfTime) return;

    // Consume the match whether or not we end up reloading the scene -- a
    // blocked-scene match is still "spent", same as upstream which returns
    // early from Statics_TimeTravel without ever re-reading the song.
    TrifoolSongEngine_ConsumeMatch();
    if (IsBlockedScene()) return;

    // Flip the age to the opposite of the saved slot, as upstream does.
    // linkAgeOnLoad is read by Scene_Init when the next entrance resolves;
    // gSaveContext.linkAge is the saved slot's age (0 = adult, 1 = child).
    gPlayState->linkAgeOnLoad = gSaveContext.linkAge ^ 1;
    gPlayState->nextEntranceIndex = gSaveContext.entranceIndex;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    // Fast fade keeps the beat snappy; vanilla pedestal uses a slower white
    // fade, but Long of Time is a free overworld flip without the Master
    // Sword cutscene, so a quick black fade reads better.
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
}

static void RegisterLongOfTimeConsumer() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickLongOfTime);
}

static RegisterShipInitFunc longOfTimeInitFunc(RegisterLongOfTimeConsumer,
                                               { CVAR_TRIFOOL_LONG_OF_TIME_AGE_FLIP });

// -----------------------------------------------------------------------------
// Menu widget.
// -----------------------------------------------------------------------------

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterLongOfTimeWidgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Long of Time flips Link's age", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_LONG_OF_TIME_AGE_FLIP)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "When the Trifool chromatic matcher recognises Long of Time, reload the current "
            "entrance with Link's age flipped (no Master Sword required). Mirrors upstream "
            "Triforce%'s Statics_TimeTravel. Skipped in the Temple of Time and the Gerudo "
            "Training Ground, matching upstream's exclusions. Requires the chromatic song "
            "engine and the Long of Time detection toggle to also be on."));
}

static RegisterMenuInitFunc longOfTimeMenuInitFunc(RegisterLongOfTimeWidgets);
