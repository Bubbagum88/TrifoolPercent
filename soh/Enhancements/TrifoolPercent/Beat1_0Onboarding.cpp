// TrifoolPercent Beat 1.0: the Onboarding Kokiri.
//
// isaac's call: "to start the game off... use the object_oE4 for some testing and to
// initiate the start of the rom hack's experience. he should be by links house and be
// interactable, with dialogue to get the ball rolling with the experience."
//
// Reuses the TfBetaKokiri actor from Beat 1.1 with params=1, so the beta skeleton +
// head + animation + head-tracking + NPC lock-on all come for free. Only the dialogue
// and spawn logic are new.
//
// Dialogue flow (text id 0x0100, 3-choice box):
//   - "Ready for the experience?"
//     [Start]       -> turn on Setup preset, Dev inputs (Arwing included),
//                      Lost Woods path, Beta Kokiri, Butterfly. Beat 1.0 flag flips so
//                      the onboarding Kokiri's follow-up text (0x0101) is a brief
//                      "see you out there".
//     [Devs only]   -> turn on Pause debug inventory and Dev inputs (Arwing + warps).
//     [Not yet]     -> no CVar changes; she stays available to talk again.
//
// All gated on CVAR_TRIFOOL_ONBOARDING (default off).

#include <array>

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
extern PlayState* gPlayState;
}

namespace {
// SCENE_KOKIRI_FOREST is 0x55 in vanilla OoT; same number in SoH's enum.
constexpr s32 kSceneKokiriForest = SCENE_KOKIRI_FOREST;

// Spawn position near Link's house in Kokiri Forest. The house sits at the north edge
// of the scene; this plants the onboarding Kokiri on the path just outside the door so
// Link cannot miss her the first time he leaves. Tune by eye in follow-up if needed.
constexpr Vec3f kSpawnPos = { -230.0f, 0.0f, -1230.0f };
constexpr s16   kSpawnRotY = 0x8000; // facing south, toward Link's path out of the house

// Text ids: 0x0100 greeting with 3-choice, 0x0101 brief "already started" follow-up.
constexpr u16 kTextIdGreeting = 0x0100;
constexpr u16 kTextIdPostStart = 0x0101;
} // namespace

static bool sOnboardingPlanted = false; // actor was spawned at least once this scene load
static bool sStarted = false;           // "Start" was picked -> we flipped the story CVars
static s16 sOnboardActorId = -1;        // alias of the Beta Kokiri actor id
static u16 sLastChoiceTextId = 0;       // msgCtx.textId we're watching for choice close

static void ResetOnboardingFlags() {
    sOnboardingPlanted = false;
    sStarted = false;
    sLastChoiceTextId = 0;
}

// -----------------------------------------------------------------------------
// Spawn the onboarding Kokiri near Link's house.
// -----------------------------------------------------------------------------

static void SpawnOnboardingKokiri() {
    if (CVarGetInteger(CVAR_TRIFOOL_ONBOARDING, 0) == 0 || gPlayState == nullptr) {
        return;
    }
    if (gPlayState->sceneNum != kSceneKokiriForest) {
        sOnboardingPlanted = false;
        return;
    }
    if (sOnboardActorId < 0) {
        sOnboardActorId = TrifoolBeat1_RegisterBetaKokiri();
    }
    if (sOnboardActorId < 0) {
        return;
    }
    // params = 1 selects the onboarding variant inside z_tf_beta_kokiri.c.
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sOnboardActorId,
                kSpawnPos.x, kSpawnPos.y, kSpawnPos.z, 0, kSpawnRotY, 0, 1);
    sOnboardingPlanted = true;
}

// -----------------------------------------------------------------------------
// Dialogue messages.
// -----------------------------------------------------------------------------
// 0x0100 is a 3-choice textbox. SoH's CustomMessage honours standard OoT control codes;
// \x1A opens a 3-choice box (TWO_CHOICE is \x1A for 2 choices; \x40 after it extends to 3).
// We use the same hand-rolled control bytes Beat 1.2 uses for the Beta Kokiri tree.

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_BOX_BREAK "\x04"
#define MSG_CHOICE_3 "\x1A\x40"   // 3-choice selector
#define MSG_COLOR_GREEN "\x05\x42"
#define MSG_COLOR_WHITE "\x05\x40"

static const char kGreetingText[] =
    "Hey, " MSG_COLOR_GREEN "hero" MSG_COLOR_WHITE "!" MSG_NEWLINE
    "Ready for the experience?" MSG_BOX_BREAK
    MSG_CHOICE_3
    "Start the adventure" MSG_NEWLINE
    "Devs only (debug + Arwing)" MSG_NEWLINE
    "Not yet" MSG_END;

static const char kPostStartText[] =
    "You're all set!" MSG_NEWLINE
    "The forest's waiting. Go find out" MSG_NEWLINE
    "where the path goes." MSG_END;

static void LoadOneMessage(const char* body, size_t bodyLen, u8 type, u8 pos,
                           bool* loadFromMessageTable) {
    std::string s(body, bodyLen);
    CustomMessage msg(s, s, s, static_cast<TextBoxType>(type), static_cast<TextBoxPosition>(pos));
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

static void LoadOnboardingMessage(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_ONBOARDING, 0) == 0) {
        return;
    }
    if (*textId == kTextIdGreeting) {
        // When Link has already picked "Start", redirect the actor's textId to the
        // post-start line instead so a second talk doesn't re-flip the CVars.
        if (sStarted) {
            *textId = kTextIdPostStart;
            LoadOneMessage(kPostStartText, sizeof(kPostStartText) - 1,
                           TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
            return;
        }
        LoadOneMessage(kGreetingText, sizeof(kGreetingText) - 1,
                       TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
        sLastChoiceTextId = kTextIdGreeting;
        return;
    }
    if (*textId == kTextIdPostStart) {
        LoadOneMessage(kPostStartText, sizeof(kPostStartText) - 1,
                       TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
    }
}

// -----------------------------------------------------------------------------
// Choice resolution.
// -----------------------------------------------------------------------------
// Poll the message context: when the 0x0100 box transitions out of the choice state
// with a picked index, apply the effect once and reset.

static void ApplyChoice(s32 choice) {
    switch (choice) {
        case 0:
            // Start the adventure: full story CVars + dev inputs for the warp menu.
            CVarSetInteger(CVAR_TRIFOOL_SETUP_PRESET, 1);
            CVarSetInteger(CVAR_TRIFOOL_DEV_INPUTS, 1);
            CVarSetInteger(CVAR_TRIFOOL_LOST_WOODS_PATH, 1);
            CVarSetInteger(CVAR_TRIFOOL_BETA_KOKIRI, 1);
            CVarSetInteger(CVAR_TRIFOOL_BUTTERFLY, 1);
            CVarSave();
            sStarted = true;
            break;
        case 1:
            // Devs only: debug inventory + Arwing + dev warps, no story CVars.
            CVarSetInteger(CVAR_TRIFOOL_PAUSE_DEBUG_INV, 1);
            CVarSetInteger(CVAR_TRIFOOL_DEV_INPUTS, 1);
            CVarSave();
            break;
        case 2:
        default:
            // Not yet: no changes. She stays by the house for next time.
            break;
    }
}

static void TickChoiceResolve() {
    if (CVarGetInteger(CVAR_TRIFOOL_ONBOARDING, 0) == 0 || gPlayState == nullptr) {
        return;
    }
    if (sLastChoiceTextId == 0) {
        return;
    }
    // The vanilla "message box has closed" state is msgMode == MSGMODE_NONE (0) and
    // textId zeroed; while the box is up msgMode is non-zero. Resolve the pick on the
    // transition out, so we read msgCtx.choiceIndex exactly once.
    if (gPlayState->msgCtx.msgMode == MSGMODE_NONE) {
        s32 choice = gPlayState->msgCtx.choiceIndex;
        ApplyChoice(choice);
        sLastChoiceTextId = 0;
    }
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolBeat1_0() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneSpawnActors>(
        SpawnOnboardingKokiri);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(LoadOnboardingMessage);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickChoiceResolve);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ResetOnboardingFlags(); });
}

static RegisterShipInitFunc beat1_0InitFunc(RegisterTrifoolBeat1_0,
                                            { CVAR_TRIFOOL_ONBOARDING });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolBeat1_0Widgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 1.0 - Onboarding Kokiri", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_ONBOARDING)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Spawns a beta Kokiri outside Link's house in Kokiri Forest. Talking opens "
            "a 3-choice box: Start the adventure (turns on Setup, Dev inputs, Lost "
            "Woods path, Beta Kokiri, Butterfly), Devs only (debug inventory + Arwing + "
            "dev warps), or Not yet. The kick-off NPC uses the same object_oE4 skeleton "
            "+ object_oE9 head combo as the Sacred Forest Meadow Beta Kokiri."));
}

static RegisterMenuInitFunc beat1_0MenuInitFunc(RegisterTrifoolBeat1_0Widgets);
