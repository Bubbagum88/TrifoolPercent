// TrifoolPercent glue: registers the Triforce% actors with ActorDB, turns the Goron Shop into the Triforce Room
// (the scene itself comes from TrifoolPercent.o2r), plays the room's cutscene and supplies its text.

#include <string>

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/ActorDB.h"
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

#define CVAR_VALUE CVarGetInteger(CVAR_TRIFOOL_TRIFORCE_ROOM, 1)

// Triforce% hijacked the Goron Shop's scene slot for its Triforce Room, and so does TrifoolPercent.
#define TRIFORCE_ROOM_SCENE SCENE_GORON_SHOP
#define TRIFORCE_ROOM_EXIT ENTR_GORON_CITY_OUTSIDE_SHOP

static s16 sTriforcePieceId = -1;
static s16 sTriforceChestId = -1;
static s16 sTriforceLightId = -1;

static s16 AddActor(const char* name, const char* desc, size_t instanceSize, ActorFunc init, ActorFunc destroy,
                    ActorFunc update, ActorFunc draw) {
    ActorDBInit entry;
    entry.name = name;
    entry.desc = desc;
    entry.category = ACTORCAT_PROP;
    entry.flags = ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    // Models load by resource path, so the actors only need the always-resident object.
    entry.objectId = OBJECT_GAMEPLAY_KEEP;
    entry.instanceSize = instanceSize;
    entry.init = init;
    entry.destroy = destroy;
    entry.update = update;
    entry.draw = draw;
    return static_cast<s16>(ActorDB::Instance->AddEntry(entry).entry.id);
}

static void AddActorsToDB() {
    if (sTriforcePieceId >= 0) {
        return;
    }
    sTriforcePieceId = AddActor("Tf_Triforce_Piece", "Triforce Piece (TrifoolPercent)", sizeof(TfTriforcePiece),
                                TfTriforcePiece_Init, TfTriforcePiece_Destroy, TfTriforcePiece_Update,
                                TfTriforcePiece_Draw);
    sTriforceChestId = AddActor("Tf_Triforce_Chest", "Triforce Chest (TrifoolPercent)", sizeof(TfTriforceChest),
                                TfTriforceChest_Init, TfTriforceChest_Destroy, TfTriforceChest_Update,
                                TfTriforceChest_Draw);
    sTriforceLightId = AddActor("Tf_Triforce_Light", "Triforce Light (TrifoolPercent)", sizeof(TfTriforceLight),
                                TfTriforceLight_Init, TfTriforceLight_Destroy, TfTriforceLight_Update,
                                TfTriforceLight_Draw);
}

static bool IsTriforceRoom() {
    return gPlayState != nullptr && gPlayState->sceneNum == TRIFORCE_ROOM_SCENE;
}

// Triforce% placed these in the room's actor list with hijacked actor ids. ActorDB ids are assigned at runtime,
// so they are spawned here instead, all at the room origin like the original (the actors position themselves).
static void SpawnTriforceRoom() {
    if (!IsTriforceRoom()) {
        return;
    }
    AddActorsToDB();

    ActorContext* actorCtx = &gPlayState->actorCtx;
    Actor_Spawn(actorCtx, gPlayState, sTriforceChestId, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
    Actor_Spawn(actorCtx, gPlayState, sTriforceLightId, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
    Actor_Spawn(actorCtx, gPlayState, sTriforcePieceId, 0.0f, 0.0f, 0.0f, 0, 0, 0, TRIFOOL_TRIFORCE_POWER);
    Actor_Spawn(actorCtx, gPlayState, sTriforcePieceId, 0.0f, 0.0f, 0.0f, 0, 0, 0, TRIFOOL_TRIFORCE_WISDOM);
    Actor_Spawn(actorCtx, gPlayState, sTriforcePieceId, 0.0f, 0.0f, 0.0f, 0, 0, 0, TRIFOOL_TRIFORCE_COURAGE);

    // Triforce% ran this as an entrance cutscene; every visit plays it here.
    gPlayState->csCtx.segment = gTrifoolTriforceRoomCs;
    gSaveContext.cutsceneTrigger = 1;
}

// Triforce% ended the cutscene with a terminator into its Ending scene. Until the Ending is ported, fade back out
// to Goron City at the same frame.
static void LeaveTriforceRoomAtCutsceneEnd() {
    if (!IsTriforceRoom()) {
        return;
    }
    CutsceneContext* csCtx = &gPlayState->csCtx;
    if (csCtx->state == CS_STATE_IDLE || csCtx->segment != gTrifoolTriforceRoomCs ||
        csCtx->frames < TRIFOOL_TRIFORCE_ROOM_CS_EXIT_FRAME || gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    gPlayState->nextEntranceIndex = TRIFORCE_ROOM_EXIT;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
}

// Text from Triforce% text/messages.c. The strings are raw OoT message bytes, so every control code below is a
// separate literal to stop a following letter being read as part of a hex escape.
#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_COLOR_WHITE "\x05\x40"
#define MSG_COLOR_GREEN "\x05\x42"
#define MSG_COLOR_YELLOW "\x05\x46"
#define MSG_SHIFT(x) "\x06" x
#define MSG_QUICKTEXT_ENABLE "\x08"
#define MSG_QUICKTEXT_DISABLE "\x09"
#define MSG_FADE(x) "\x0E" x
#define MSG_TEXT_SPEED(x) "\x14" x
#define MSG_UNSKIPPABLE "\x1A"
#define MSG_THREE_CHOICE "\x1C"

#define GODDESS_TEXT(speed) MSG_UNSKIPPABLE MSG_COLOR_YELLOW MSG_TEXT_SPEED(speed)

// sizeof keeps embedded NUL bytes (TEXT_SPEED 0 in 0x0E06) that strlen would cut off.
#define RAW_MESSAGE(id, literal) { id, std::string(literal, sizeof(literal) - 1) }

struct TrifoolMessage {
    u16 textId;
    std::string text;
};

static const TrifoolMessage sTriforceRoomMessages[] = {
    // Triforce% also showed the beta Triforce icon (ITEM_ICON 0x73, a patched heart piece icon). Not ported yet.
    RAW_MESSAGE(0x0ACE, MSG_UNSKIPPABLE MSG_QUICKTEXT_ENABLE "You got the " MSG_COLOR_YELLOW "Triforce" MSG_COLOR_WHITE
                "!!!" MSG_QUICKTEXT_DISABLE MSG_NEWLINE "After 23 years, the dreams of" MSG_NEWLINE
                "millions of players around the" MSG_NEWLINE "world have finally come true!" MSG_END),
    RAW_MESSAGE(0x0E00, GODDESS_TEXT("\x02") MSG_SHIFT("\x4F") "Link..." MSG_FADE("\x18") MSG_END),
    RAW_MESSAGE(0x0E01, GODDESS_TEXT("\x01") MSG_SHIFT("\x52") "Link." MSG_FADE("\x28") MSG_END),
    RAW_MESSAGE(0x0E02, GODDESS_TEXT("\x01") "It is we, the Golden Goddesses" MSG_NEWLINE "of the Triforce:" MSG_NEWLINE
                "Din, Nayru, and Farore..." MSG_FADE("\x38") MSG_END),
    RAW_MESSAGE(0x0E03, GODDESS_TEXT("\x01") "You have done well to arrive" MSG_NEWLINE "here, and your heart is in"
                MSG_NEWLINE "balance..." MSG_FADE("\x38") MSG_END),
    RAW_MESSAGE(0x0E04, GODDESS_TEXT("\x01") "You are now the master of the" MSG_NEWLINE "complete Triforce..."
                MSG_FADE("\x38") MSG_END),
    RAW_MESSAGE(0x0E05, GODDESS_TEXT("\x01") "The True Force stirs within you..." MSG_FADE("\x38") MSG_END),
    RAW_MESSAGE(0x0E06, GODDESS_TEXT("\x01") "Now, Link... What do you wish for?" MSG_NEWLINE MSG_TEXT_SPEED("\x00")
                MSG_THREE_CHOICE MSG_COLOR_GREEN "Be King of Hyrule" MSG_NEWLINE "Rupees" MSG_NEWLINE
                "See the future" MSG_END),
    RAW_MESSAGE(0x0E07, GODDESS_TEXT("\x01") MSG_SHIFT("\x13") "Your wish has been granted." MSG_FADE("\x46") MSG_END),
};

static void LoadTriforceRoomMessage(u16* textId, bool* loadFromMessageTable) {
    if (!IsTriforceRoom()) {
        return;
    }
    for (const TrifoolMessage& message : sTriforceRoomMessages) {
        if (message.textId == *textId) {
            CustomMessage msg(message.text, message.text, message.text, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE);
            msg.LoadIntoFont();
            *loadFromMessageTable = false;
            return;
        }
    }
}

static void RegisterTrifoolTriforceRoom() {
    COND_HOOK(OnSceneSpawnActors, CVAR_VALUE, SpawnTriforceRoom);
    COND_HOOK(OnPlayerUpdate, CVAR_VALUE, LeaveTriforceRoomAtCutsceneEnd);
    // One unfiltered hook: COND_ID_HOOK keeps a single static hook id per call site, so registering it in a loop
    // would leave only the last text id hooked. LoadTriforceRoomMessage checks the id itself.
    COND_HOOK(OnOpenText, CVAR_VALUE, LoadTriforceRoomMessage);
}

static RegisterShipInitFunc initFunc(RegisterTrifoolTriforceRoom, { CVAR_TRIFOOL_TRIFORCE_ROOM });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolPercentWidgets() {
    SohGui::mSohMenu->AddSidebarEntry("Enhancements", "TrifoolPercent", 1);
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Triforce Room", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TRIFORCE_ROOM)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "Turns the Goron Shop into the Triforce% Triforce Room: the Triforce pieces, chest and light, the "
            "\"You got the Triforce!!!\" cutscene and the goddesses' wish. Needs TrifoolPercent.o2r in the mods "
            "folder for the room itself."));
}

static RegisterMenuInitFunc menuInitFunc(RegisterTrifoolPercentWidgets);
