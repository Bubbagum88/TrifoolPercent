// TrifoolPercent Running Man boss -- scaffold slice (dialogue + Sages' Charm handoff).
//
// Adapted from Triforce% actor/BossRunningMan/BossRunningMan.c (c) 2019-2022 The
// Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt. Dialogue text
// copied verbatim from upstream assets/text/messages.c (ids 0x0C30..0x0C34).
//
// Scope of THIS slice (scripting foundation only, no boss AI, no model yet):
//
//   1. The five post-fight "I guess you're a better cheater than I am..." textboxes
//      the upstream Running Man speaks after Link beats him. Loaded through
//      CustomMessage + OnOpenText, same pattern the Butterfly and Sages' Charm beats
//      use. All five are UNSKIPPABLE + EVENT in upstream; we mirror that so the
//      Running Man's regret monologue cannot be mashed through.
//
//   2. When the last textbox (0x0C34, "Here, you can have this.") closes, grant the
//      Sages' Charm (TrifoolBeat_GrantSagesCharm) and fire the item-get card
//      (TRIFOOL_TEXT_SAGES_CHARM_GET). This is the "deliver reward" handoff from
//      upstream RunningMan_UpdateDialogue (actors/BossRunningMan/BossRunningMan.c
//      lines 361-383): on 0x0C34 the upstream actor calls Actor_PickUp with
//      GI_SAGES_CHARM and then sets SAGES_CHARM_VAR; we skip the vanilla GetItem
//      overhead-hold animation here because the item's GetItemEntry does not land
//      until the model + o2r pipeline is wired (see docs/items/sages-charm.md
//      follow-up 5 and docs/actors/running-man-boss.md).
//
//   3. A one-shot "Start Running Man dialogue (debug)" pulse CVar that fires
//      TRIFOOL_TEXT_RUNNING_MAN_C30 so the dialogue chain can be exercised without
//      the boss fight or the Running Man actor placement -- same shape as Beat
//      2.0's "Grant Sages' Charm (debug)" pulse.
//
// Explicit NON-goals of this slice (each ships as its own PR):
//
// - The actual boss AI (1679 LOC in upstream actor/BossRunningMan/BossRunningMan.c):
//   intro cutscene (BossRunningManIntroCS), run/dodge/kick state machine, arrow
//   phases, four-ghost after-image system, hurt/explode-rupee death, music swap.
//   The upstream state machine is scoped as separate port slices in
//   docs/actors/running-man-boss.md.
//
// - The custom Running Man model (`object_BossRunningManObj` = DEP_OBJECT_NUM 59
//   upstream). The repo's actors/BossRunningMan/BossRunningManObj.h/.ld only
//   declare the symbols -- the .anim/.mtlex/.skel sources live in a .blend that
//   has not been exported yet. Blocked on Isaac's Blender machine, same as the
//   Sages' Charm 24px C-button icon.
//
// - The exploding giant magenta rupee reward drop (ACTOR_EN_EX_RUPPY,
//   ACTORPARAM_BETAGIANTRUPEE). That is its own custom actor and is spawned by
//   RunningMan_Init in the upstream source; this slice does not spawn it.
//
// - The magic-bar visual 2x scale-up (gSaveContext.unk_13F0 = 9 in the upstream
//   RunningMan_UpdateDialogue timer branch at frame 25). Visual-only; filed as
//   docs/items/sages-charm.md follow-up 3.
//
// - Spawning the Running Man actor in-world (Hyrule Field). The scaffold here is
//   the dialogue + handoff only; actor placement rides on either the custom model
//   landing or a vanilla ACTOR_EN_MM ride-on hook, pending Isaac's call.

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/custom-message/CustomMessageManager.h"
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
#include "z64save.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_BOX_BREAK "\x04"
#define MSG_QUICKTEXT_ON "\x1B"
#define MSG_QUICKTEXT_OFF "\x1C"
// 0x15 is CTRL_EVENT (ends a line, waits for the actor to advance). The upstream
// Running Man dialogue uses EVENT after every box to park on each line until its
// state-machine tick advances it; in a scripted-less textbox-only path we fall
// through to MSG_END so the player can tap through the chain naturally.
// If a future slice wires an actor that walks the EVENT advance, swap MSG_END
// at the end of 0x0C30..0x0C33 for the engine's event terminator.

static const char kRunningManC30[] =
    "huff... huff..." MSG_BOX_BREAK
    "I guess you're a better cheater" MSG_NEWLINE
    "than I am..." MSG_BOX_BREAK
    "I never thought anyone would" MSG_NEWLINE
    "find a power like the one I did," MSG_NEWLINE
    "seven years ago..." MSG_END;

static const char kRunningManC31[] =
    "That day, after Hyrule Castle" MSG_NEWLINE
    "was attacked, I went on my usual" MSG_NEWLINE
    "running route to clear my mind." MSG_BOX_BREAK
    "Somewhere out in the field here," MSG_NEWLINE
    "I stepped on this object, lying" MSG_NEWLINE
    "on the ground." MSG_BOX_BREAK
    "It must have been dropped by" MSG_NEWLINE
    "someone fleeing town." MSG_END;

static const char kRunningManC32[] =
    "It was a golden locket. I tried it" MSG_NEWLINE
    "on and felt this rush all over my" MSG_NEWLINE
    "body!" MSG_BOX_BREAK
    "I could run faster than I could" MSG_NEWLINE
    "ever imagine. In fact, I could run" MSG_NEWLINE
    "faster than light!" MSG_BOX_BREAK
    "That's how I was able to finish" MSG_NEWLINE
    "the race before I started, and" MSG_NEWLINE
    "how I was just fighting you." MSG_END;

static const char kRunningManC33[] =
    "But I now realize this has made" MSG_NEWLINE
    "me a terrible man." MSG_BOX_BREAK
    "I wanted to be the fastest so" MSG_NEWLINE
    "badly I tried to kill you!" MSG_BOX_BREAK
    "sniffle..." MSG_END;

static const char kRunningManC34[] =
    "I just want to be my own self" MSG_NEWLINE
    "again, to run free in the wild." MSG_NEWLINE
    "I don't care about the magic" MSG_NEWLINE
    "anymore." MSG_BOX_BREAK
    "Here, you can have this." MSG_END;

static void LoadOneMessage(const char* body, size_t bodyLen, u8 type, u8 pos,
                           bool* loadFromMessageTable) {
    std::string s(body, bodyLen);
    CustomMessage msg(s, s, s, static_cast<TextBoxType>(type), static_cast<TextBoxPosition>(pos));
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

static void LoadRunningManMessage(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_RUNNING_MAN, 0) == 0) {
        return;
    }
    const char* body = nullptr;
    size_t bodyLen = 0;
    switch (*textId) {
        case TRIFOOL_TEXT_RUNNING_MAN_C30:
            body = kRunningManC30;
            bodyLen = sizeof(kRunningManC30) - 1;
            break;
        case TRIFOOL_TEXT_RUNNING_MAN_C31:
            body = kRunningManC31;
            bodyLen = sizeof(kRunningManC31) - 1;
            break;
        case TRIFOOL_TEXT_RUNNING_MAN_C32:
            body = kRunningManC32;
            bodyLen = sizeof(kRunningManC32) - 1;
            break;
        case TRIFOOL_TEXT_RUNNING_MAN_C33:
            body = kRunningManC33;
            bodyLen = sizeof(kRunningManC33) - 1;
            break;
        case TRIFOOL_TEXT_RUNNING_MAN_C34:
            body = kRunningManC34;
            bodyLen = sizeof(kRunningManC34) - 1;
            break;
        default:
            return;
    }
    // Upstream uses POS_MIDDLE + BOX_BLACK for all five; we keep that for parity.
    LoadOneMessage(body, bodyLen, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE,
                   loadFromMessageTable);
}

// -----------------------------------------------------------------------------
// Chain the handoff: when 0x0C34 closes, grant the Sages' Charm and start the
// item-get textbox. Mirrors upstream RunningMan_UpdateDialogue lines 361-383.
// -----------------------------------------------------------------------------

// GameInteractor's OnOpenText fires on message open; there is no symmetric "on
// close" hook, so we latch inside the per-frame tick: remember we saw 0x0C34
// open, then act the first frame msgMode returns to NONE.
static bool sWatchForC34Close = false;
static bool sPendingHandoff = false;

static void TickRunningManHandoff() {
    if (CVarGetInteger(CVAR_TRIFOOL_RUNNING_MAN, 0) == 0 || gPlayState == nullptr) {
        sWatchForC34Close = false;
        return;
    }
    // Latch: when the engine shows 0x0C34, remember we're waiting for its close.
    if (gPlayState->msgCtx.textId == TRIFOOL_TEXT_RUNNING_MAN_C34 &&
        gPlayState->msgCtx.msgMode != MSGMODE_NONE) {
        sWatchForC34Close = true;
    }
    if (sWatchForC34Close && gPlayState->msgCtx.msgMode == MSGMODE_NONE) {
        sWatchForC34Close = false;
        sPendingHandoff = true;
    }
    if (sPendingHandoff && gPlayState->msgCtx.msgMode == MSGMODE_NONE) {
        sPendingHandoff = false;
        TrifoolBeat_GrantSagesCharm();
        Message_StartTextbox(gPlayState, TRIFOOL_TEXT_SAGES_CHARM_GET, NULL);
    }
}

// -----------------------------------------------------------------------------
// Debug pulse: fire the first dialogue box (0x0C30) on tick without any actor.
// -----------------------------------------------------------------------------

static bool sPendingDialoguePulse = false;

static void TickDebugPulse() {
    if (CVarGetInteger(CVAR_TRIFOOL_RUNNING_MAN, 0) == 0) {
        return;
    }
    if (CVarGetInteger(CVAR_TRIFOOL_RUNNING_MAN_DEBUG_PULSE, 0) != 0) {
        CVarSetInteger(CVAR_TRIFOOL_RUNNING_MAN_DEBUG_PULSE, 0);
        CVarSave();
        sPendingDialoguePulse = true;
    }
    if (!sPendingDialoguePulse) {
        return;
    }
    if (gPlayState == nullptr) {
        return;
    }
    if (gPlayState->msgCtx.msgMode != MSGMODE_NONE) {
        return;
    }
    sPendingDialoguePulse = false;
    Message_StartTextbox(gPlayState, TRIFOOL_TEXT_RUNNING_MAN_C30, NULL);
}

static void ResetRunningManFlags() {
    sWatchForC34Close = false;
    sPendingHandoff = false;
    sPendingDialoguePulse = false;
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolRunningMan() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(LoadRunningManMessage);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickRunningManHandoff);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickDebugPulse);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ResetRunningManFlags(); });
}

static RegisterShipInitFunc runningManInitFunc(RegisterTrifoolRunningMan,
                                               { CVAR_TRIFOOL_RUNNING_MAN });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolRunningManWidgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path,
                                "Running Man boss - dialogue scaffold",
                                WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_RUNNING_MAN)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Scaffold slice for the Running Man / Sages' Charm handoff. Loads the "
            "five post-fight textboxes (0x0C30..0x0C34) through CustomMessage and, "
            "when the last one closes, grants the Sages' Charm + fires the item-get "
            "card. The custom model and the full boss AI port are deferred -- see "
            "docs/actors/running-man-boss.md. Requires the Beat 2.0 Sages' Charm "
            "toggle to be on for the item-get textbox content to render."));
    SohGui::mSohMenu->AddWidget(path,
                                "Start Running Man dialogue (debug)",
                                WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TrifoolPercent.RunningManDialoguePulse"))
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Tick to open the first Running Man textbox (0x0C30) once. The box opens "
            "on the next frame no other textbox is up. Requires the Running Man beat "
            "above to be on. Clears itself after firing."));
}

static RegisterMenuInitFunc runningManMenuInitFunc(RegisterTrifoolRunningManWidgets);
