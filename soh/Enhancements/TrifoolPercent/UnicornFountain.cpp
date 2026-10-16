// TrifoolPercent Unicorn Fountain glue: Overture-of-Sages dialogue ported from Triforce% text/messages.c,
// and the end-of-visit scene transition (fade back to Kokiri Forest when the fairy's fall ends).
//
// The scene itself (Fast64 export of UnicornFountain.blend) lives in SCENE_TEST01 and already loads
// through TrifoolPercent.o2r. The fairy actor is spawned by TrifoolPercent.cpp's SpawnUnicornFountain
// (unchanged); this file hooks around it to add the pieces upstream relied on from statics/statics.c
// and the scene's own cutscene.
//
// Dialogue text ids (0x0D23, 0x0074, 0x0894, 0x0871) come straight from Triforce% text/messages.c
// (c) 2019-2022 The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt.
//
// Scope note: this slice stops short of actually granting the Overture of Sages song (upstream's
// Ocarina_GiveOvertureOfSages patches the vanilla song table and depends on music/overtureofsages.aseq,
// neither of which is ported yet). The "You have learned the Overture of Sages" textbox stands in
// for the song gift until that beat lands.

#include <string>

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/custom-message/CustomMessageManager.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "TrifoolPercent.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define CVAR_FOUNTAIN_VALUE CVarGetInteger(CVAR_TRIFOOL_UNICORN_FOUNTAIN, 1)
#define UNICORN_FOUNTAIN_SCENE SCENE_TEST01

// Where Link lands when the fountain visit ends. Matches the "test harness" convention the Triforce
// Room already uses: every visit returns to Kokiri Forest by Link's house, so the hex hub's doorway
// 4 (Unicorn Fountain) and vanilla Kokiri Forest round-trip cleanly.
#define UNICORN_FOUNTAIN_EXIT ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE

// Frames to hang after the fairy's fall ends before firing the fade. Lets the sparkle particles die
// out and matches the brief hold upstream's cutscene has before its own scene transition.
#define EXIT_HOLD_FRAMES 20
static s32 sExitHoldFrames = 0;

static bool IsUnicornFountain() {
    return gPlayState != nullptr && gPlayState->sceneNum == UNICORN_FOUNTAIN_SCENE;
}

// -----------------------------------------------------------------------------
// Overture-of-Sages dialogue (Triforce% text/messages.c, verbatim bytes).
// -----------------------------------------------------------------------------

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_COLOR_WHITE "\x05\x40"
#define MSG_COLOR_LIGHTBLUE "\x05\x44"
#define MSG_COLOR_YELLOW "\x05\x46"
#define MSG_SHIFT(x) "\x06" x
#define MSG_QUICKTEXT_ENABLE "\x08"
#define MSG_QUICKTEXT_DISABLE "\x09"
#define MSG_UNSKIPPABLE "\x1A"
#define MSG_OCARINA "\x7F"

#define RAW_MESSAGE(id, literal) { id, std::string(literal, sizeof(literal) - 1) }

struct FountainMessage {
    u16 textId;
    std::string text;
    u8 textboxType;
    u8 textboxPos;
};

// Upstream pos VARIABLE / BOX_OCARINA land SoH-side as TEXTBOX_TYPE_OCARINA; BOX_BLACK + POS_MIDDLE
// is TEXTBOX_TYPE_BLACK + TEXTBOX_POS_MIDDLE; BOX_BLUE is TEXTBOX_TYPE_BLUE.
static const FountainMessage sFountainMessages[] = {
    // 0x0D23: "Play this song before the blade of destiny, and the path of the Sages shall be revealed."
    RAW_MESSAGE(0x0D23, MSG_UNSKIPPABLE "Play this song before the blade of" MSG_NEWLINE
                        "destiny, and the path of the Sages" MSG_NEWLINE
                        "shall be revealed." MSG_END),
    // 0x0074: "You have learned the Overture of Sages!!"
    RAW_MESSAGE(0x0074, MSG_UNSKIPPABLE MSG_SHIFT("\x28") "You have learned the" MSG_NEWLINE
                        MSG_SHIFT("\x29") MSG_COLOR_LIGHTBLUE "Overture of Sages" MSG_COLOR_WHITE "!!" MSG_END),
    // 0x0871: "Play the Overture of Sages!"
    RAW_MESSAGE(0x0871, MSG_QUICKTEXT_ENABLE MSG_SHIFT("\x11") "Play the " MSG_COLOR_LIGHTBLUE
                        "Overture of Sages" MSG_COLOR_WHITE "!" MSG_NEWLINE MSG_NEWLINE MSG_NEWLINE
                        MSG_QUICKTEXT_DISABLE MSG_OCARINA MSG_END),
    // 0x0894: "You played the Overture of Sages."
    RAW_MESSAGE(0x0894, MSG_QUICKTEXT_ENABLE MSG_SHIFT("\x01") "You played the " MSG_COLOR_LIGHTBLUE
                        "Overture of Sages" MSG_COLOR_WHITE "." MSG_NEWLINE MSG_NEWLINE MSG_NEWLINE
                        MSG_QUICKTEXT_DISABLE MSG_END),
};

static void LoadFountainMessage(u16* textId, bool* loadFromMessageTable) {
    if (!IsUnicornFountain() || CVAR_FOUNTAIN_VALUE == 0) {
        return;
    }
    for (const FountainMessage& message : sFountainMessages) {
        if (message.textId == *textId) {
            // Upstream used BOX_BLUE for 0x0074 ("Overture of Sages learned") and BOX_BLACK for the rest.
            // SoH's blue variant has a different name across forks, so everything renders through BLACK
            // here; a follow-up slice can swap in the blue style once we standardize on the right enum.
            CustomMessage msg(message.text, message.text, message.text, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE);
            msg.LoadIntoFont();
            *loadFromMessageTable = false;
            return;
        }
    }
}

// -----------------------------------------------------------------------------
// Scene transition out, mirroring LeaveTriforceRoomAtCutsceneEnd's shape.
// -----------------------------------------------------------------------------

static void ResetFountainStateOnEntry() {
    // Called once per scene load (OnSceneSpawnActors). The fairy actor also clears its own done
    // flag in Init; this covers the window between scene load and the fairy's first update.
    if (IsUnicornFountain()) {
        TrifoolUnicornFountain_ClearFairyDone();
        sExitHoldFrames = 0;
    }
}

static void LeaveUnicornFountainWhenFairyDone() {
    if (!IsUnicornFountain() || CVAR_FOUNTAIN_VALUE == 0) {
        return;
    }
    if (!TrifoolUnicornFountain_IsFairyDone()) {
        sExitHoldFrames = 0;
        return;
    }
    if (gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    if (++sExitHoldFrames < EXIT_HOLD_FRAMES) {
        return;
    }
    gPlayState->nextEntranceIndex = UNICORN_FOUNTAIN_EXIT;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
    TrifoolUnicornFountain_ClearFairyDone();
}

// -----------------------------------------------------------------------------
// Registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolUnicornFountainExtras() {
    COND_HOOK(OnOpenText, CVAR_FOUNTAIN_VALUE, LoadFountainMessage);
    COND_HOOK(OnSceneSpawnActors, CVAR_FOUNTAIN_VALUE, ResetFountainStateOnEntry);
    COND_HOOK(OnPlayerUpdate, CVAR_FOUNTAIN_VALUE, LeaveUnicornFountainWhenFairyDone);
}

static RegisterShipInitFunc fountainExtrasInitFunc(RegisterTrifoolUnicornFountainExtras,
                                                   { CVAR_TRIFOOL_UNICORN_FOUNTAIN });
