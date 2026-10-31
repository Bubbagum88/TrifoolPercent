// TrifoolPercent glue: registers the Triforce% actors with ActorDB, sets up the Triforce Room
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
#define CVAR_FOUNTAIN_VALUE CVarGetInteger(CVAR_TRIFOOL_UNICORN_FOUNTAIN, 1)
#define CVAR_FOUNTAIN_OVERWORLD_VALUE CVarGetInteger(CVAR_TRIFOOL_FOUNTAIN_OVERWORLD, 0)
#define CVAR_DOT_ADULT_VALUE CVarGetInteger(CVAR_TRIFOOL_DOT_ADULT, 0)

// Triforce% hijacked the Goron Shop's scene slot for its Triforce Room. TrifoolPercent uses the unused debug scene
// Besitu instead (map select "116:Test Room"), so the Goron Shop stays vanilla.
#define TRIFORCE_ROOM_SCENE SCENE_BESITU
// For testing, the room sends Link back out next to his house, where a loading zone leads into it (see below).
#define TRIFORCE_ROOM_EXIT ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE
#define TRIFORCE_ROOM_ENTRANCE ENTR_BESITU_0

// Unicorn Fountain lands in the Test01 debug slot (map select "115:Test Map 1"), the next unused slot after Besitu.
#define UNICORN_FOUNTAIN_SCENE SCENE_TEST01
#define UNICORN_FOUNTAIN_ENTRANCE ENTR_TEST01_0

#define CVAR_TEST_ENTRANCE_VALUE CVarGetInteger(CVAR_TRIFOOL_TEST_ENTRANCE, 1)

// Single test loading zone in Kokiri Forest, by Link's house, that warps into the Test Room
// debug hub (SCENE_SYOTES, "118:Syotes" in SoH's map select). This replaces the two POC-era
// flames that led to the Triforce Room and the Unicorn Fountain directly: those scenes are
// now reached from inside the hub instead, so testing is one keypress away whichever room
// is being iterated on.
//
// Visually marked with a custom TfUnusedFlame actor driven by vanilla OoT's unused
// OBJECT_FIRE slot (0x0F, TCRF "Dancing Flame"). The slot ships gFire0Tex..gFire7Tex
// as an eight-frame ia8 32x64 loop but no retail actor references it; this one does.
// Falls back on a dynamic blue point light if the extract pipeline leaves object_fire
// out, so the warp spot is still legible either way.
static Vec3f sTestEntrancePos = { -18.0f, -80.0f, 816.0f };
#define TEST_ENTRANCE_RADIUS 40.0f
#define TEST_ENTRANCE_HEIGHT 40.0f

// SCENE_SYOTES (0x68) ships in vanilla as an unused "test room" slot. TrifoolPercent's hex
// Test Room hub lives there (PR #30 lands the actual geometry); the entrance id below works
// against the vanilla scene too, so this flame does not depend on the hub being installed.
#define TEST_ROOM_ENTRANCE ENTR_SYOTES_0

static s16 sTriforcePieceId = -1;
static s16 sTriforceChestId = -1;
static s16 sTriforceLightId = -1;
static s16 sBetaGreatFairyId = -1;
static s16 sUnusedFlameId = -1;
static s16 sMimicChestId = -1;

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

static void AddFountainActorsToDB() {
    if (sBetaGreatFairyId >= 0) {
        return;
    }
    sBetaGreatFairyId = AddActor("Tf_Beta_Great_Fairy", "Beta Great Fairy (TrifoolPercent)", sizeof(TfBetaGreatFairy),
                                 TfBetaGreatFairy_Init, TfBetaGreatFairy_Destroy, TfBetaGreatFairy_Update,
                                 TfBetaGreatFairy_Draw);
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

    // Debug scenes have no place name, so the room brings its own title card (built by make_title_card.py).
    TitleCard_InitPlaceName(gPlayState, &gPlayState->actorCtx.titleCtx, (void*)gTrifoolTriforceRoomTitleCardTex, 160,
                            120, 144, 24, 20);

    // Triforce% ran this as an entrance cutscene; every visit plays it here.
    gPlayState->csCtx.segment = gTrifoolTriforceRoomCs;
    gSaveContext.cutsceneTrigger = 1;
}

// Triforce% ended the cutscene with a terminator into its Ending scene. Until the Ending is ported, fade back out
// to Kokiri Forest at the same frame.
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

static bool IsKokiriForest() {
    return gPlayState != nullptr && gPlayState->sceneNum == SCENE_KOKIRI_FOREST;
}

static bool IsUnicornFountain() {
    return gPlayState != nullptr && gPlayState->sceneNum == UNICORN_FOUNTAIN_SCENE;
}

// Triforce% placed the fairy as part of the room's actor list; we spawn her from code so the actor id can be
// assigned at runtime. The actor positions itself from its home.pos, so it needs the room origin.
static void SpawnUnicornFountain() {
    if (!IsUnicornFountain()) {
        return;
    }
    AddFountainActorsToDB();

    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sBetaGreatFairyId, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);

    // Debug scenes have no place name, so the fountain brings its own title card.
    TitleCard_InitPlaceName(gPlayState, &gPlayState->actorCtx.titleCtx, (void*)gTrifoolUnicornFountainTitleCardTex,
                            160, 120, 144, 24, 20);
}

static void AddUnusedFlameActorToDB() {
    if (sUnusedFlameId >= 0) {
        return;
    }
    sUnusedFlameId = AddActor("Tf_Unused_Flame", "Unused Flame (TrifoolPercent)", sizeof(TfUnusedFlame),
                              TfUnusedFlame_Init, TfUnusedFlame_Destroy, TfUnusedFlame_Update, TfUnusedFlame_Draw);
}

// Exposed for other TrifoolPercent translation units (e.g. TestRoom.cpp) that want to drop a
// dev-convenience flame without duplicating the ActorDB registration.
extern "C" void TrifoolSpawnUnusedFlame(float x, float y, float z) {
    if (gPlayState == nullptr) {
        return;
    }
    AddUnusedFlameActorToDB();
    if (sUnusedFlameId < 0) {
        return;
    }
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sUnusedFlameId, x, y, z, 0, 0, 0, 0);
}

static void AddMimicChestActorToDB() {
    if (sMimicChestId >= 0) {
        return;
    }
    sMimicChestId = AddActor("Tf_Mimic_Chest", "Mimic Chest (TrifoolPercent)", sizeof(TfMimicChest),
                             TfMimicChest_Init, TfMimicChest_Destroy, TfMimicChest_Update, TfMimicChest_Draw);
}

// Spawn position inside SCENE_SYOTES (the hex Test Room hub). Puts the Mimic Chest in the
// middle of the room so Link can walk around it, trigger the lurch, then back off to see
// the reset. Tune in-game.
#define CVAR_MIMIC_CHEST_VALUE CVarGetInteger(CVAR_TRIFOOL_MIMIC_CHEST, 0)
static Vec3f sMimicChestSpawnPos = { 0.0f, 0.0f, 0.0f };

static bool IsTestRoomSceneForMimic() {
    return gPlayState != nullptr && gPlayState->sceneNum == SCENE_SYOTES;
}

static void SpawnMimicChestInTestRoom() {
    if (!IsTestRoomSceneForMimic()) {
        return;
    }
    AddMimicChestActorToDB();
    if (sMimicChestId < 0) {
        return;
    }
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sMimicChestId, sMimicChestSpawnPos.x,
                sMimicChestSpawnPos.y, sMimicChestSpawnPos.z, 0, 0, 0, 0);
}

static void SpawnTestEntranceMarker() {
    if (!IsKokiriForest()) {
        return;
    }
    AddUnusedFlameActorToDB();
    if (sUnusedFlameId < 0) {
        return;
    }
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sUnusedFlameId, sTestEntrancePos.x, sTestEntrancePos.y,
                sTestEntrancePos.z, 0, 0, 0, 0);
}

static bool PlayerAt(const Vec3f* pos) {
    Player* player = GET_PLAYER(gPlayState);
    f32 dy = player->actor.world.pos.y - pos->y;
    return dy >= -TEST_ENTRANCE_HEIGHT && dy <= TEST_ENTRANCE_HEIGHT &&
           Math_Vec3f_DistXZ(&player->actor.world.pos, (Vec3f*)pos) <= TEST_ENTRANCE_RADIUS;
}

static void EnterTestRoomFromFlame() {
    if (!IsKokiriForest() || gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    if (!PlayerAt(&sTestEntrancePos)) {
        return;
    }
    gPlayState->nextEntranceIndex = TEST_ROOM_ENTRANCE;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
}

// Overworld entrance to the Unicorn Fountain, at the bottom of Zora's Domain. Triforce%
// repurposed the vanilla Zora's Fountain entrance (0x0560) by patching the entrance table so
// that walking out of Zora's Domain after the Fire Medallion thawed the ice landed Link in
// the Unicorn Fountain; the fan-rumored "secret entrance" behind a basin wall at the bottom
// of SCENE_ZORAS_DOMAIN is where that reroute pointed. We take the native path instead -- a
// cvar-gated TfUnusedFlame on that same basin wall spot that warps Link into the fountain
// scene, leaving the vanilla entrance table untouched. Coordinates are provisional (back of
// the Domain, near the basin floor); tune in-game.
static Vec3f sFountainOverworldPos = { -40.0f, -920.0f, -1950.0f };

static bool IsZorasDomain() {
    return gPlayState != nullptr && gPlayState->sceneNum == SCENE_ZORAS_DOMAIN;
}

static void SpawnFountainOverworldMarker() {
    if (!IsZorasDomain()) {
        return;
    }
    AddUnusedFlameActorToDB();
    if (sUnusedFlameId < 0) {
        return;
    }
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sUnusedFlameId, sFountainOverworldPos.x,
                sFountainOverworldPos.y, sFountainOverworldPos.z, 0, 0, 0, 0);
}

static void EnterFountainFromOverworld() {
    if (!IsZorasDomain() || gPlayState->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }
    if (!PlayerAt(&sFountainOverworldPos)) {
        return;
    }
    gPlayState->nextEntranceIndex = UNICORN_FOUNTAIN_ENTRANCE;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
}

// Door of Time adult-side fix. Vanilla OoT animates the Door of Time open in the Temple
// of Time intro cutscene via Demo_Kankyo (DEMOKANKYO_DOOR_OF_TIME): the Init path
// branches on EVENTCHKINF_OPENED_THE_DOOR_OF_TIME (0x4B), spawning Door_Toki (closed, with
// collision) when the flag is clear and setting roomCtx.unk_74[1] = 0xFF (the "door
// opened" room-draw variant) when the flag is set. The opening animation itself is only
// scheduled when the cutscene fires actor cue 2 (CutsceneFlags_Get(play, 2) in
// DemoKankyo_SetupType), which does not happen for adult Link on a fresh warp into
// SCENE_TEMPLE_OF_TIME -- so adult sees whatever state is saved. For Setup / dev-warps
// (and for any save where the vanilla opening cutscene never ran as child) that means a
// closed door blocking adult Link.
//
// We patch three things on adult entry, idempotently: set the eventchkinf flag (future
// Door_Toki updates and any other door-state queries then read "open"); overwrite
// roomCtx.unk_74[1] = 0xFF so the Temple of Time scene draw-config picks the
// opened-door room mesh for this frame on; and the already-spawned Door_Toki actor's own
// Update will drop its collision as soon as it next ticks because its only gate is the
// same eventchkinf we just set. No change needed in Door_Toki itself.
static void FixDoorOfTimeOnAdultEntry() {
    if (gPlayState == nullptr || gPlayState->sceneNum != SCENE_TEMPLE_OF_TIME) {
        return;
    }
    if (!LINK_IS_ADULT) {
        return;
    }
    if (!GET_EVENTCHKINF(EVENTCHKINF_OPENED_THE_DOOR_OF_TIME)) {
        SET_EVENTCHKINF(EVENTCHKINF_OPENED_THE_DOOR_OF_TIME);
    }
    // Flip the room-draw variant to the open-door mesh. Demo_Kankyo would have done this
    // itself if its Init had seen the flag set, so we are catching up for the opening-
    // cutscene path adult Link never ran. Shipwright keeps the N64 field name roomCtx.unk_74.
    gPlayState->roomCtx.unk_74[1] = 0xFF;
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

static void RegisterTrifoolTestEntrance() {
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ENTRANCE_VALUE, SpawnTestEntranceMarker);
    COND_HOOK(OnPlayerUpdate, CVAR_TEST_ENTRANCE_VALUE, EnterTestRoomFromFlame);
}

static void RegisterTrifoolUnicornFountain() {
    COND_HOOK(OnSceneSpawnActors, CVAR_FOUNTAIN_VALUE, SpawnUnicornFountain);
}

static void RegisterTrifoolFountainOverworld() {
    COND_HOOK(OnSceneSpawnActors, CVAR_FOUNTAIN_OVERWORLD_VALUE, SpawnFountainOverworldMarker);
    COND_HOOK(OnPlayerUpdate, CVAR_FOUNTAIN_OVERWORLD_VALUE, EnterFountainFromOverworld);
}

static void RegisterTrifoolDoorOfTimeAdult() {
    COND_HOOK(OnSceneSpawnActors, CVAR_DOT_ADULT_VALUE, FixDoorOfTimeOnAdultEntry);
}

static void RegisterTrifoolMimicChest() {
    // SYOTES's exported actor list is empty, and OnSceneSpawnActors is gated on
    // play->numSetupActors != 0 (z_actor.c), so hooks keyed to that never fire for the hex
    // Test Room. OnSceneInit does, after the scene is loaded; same requirement as TestRoom.cpp.
    COND_HOOK(OnSceneInit, CVAR_MIMIC_CHEST_VALUE, [](int16_t) { SpawnMimicChestInTestRoom(); });
}

static RegisterShipInitFunc initFunc(RegisterTrifoolTriforceRoom, { CVAR_TRIFOOL_TRIFORCE_ROOM });
static RegisterShipInitFunc testEntranceInitFunc(RegisterTrifoolTestEntrance, { CVAR_TRIFOOL_TEST_ENTRANCE });
static RegisterShipInitFunc fountainInitFunc(RegisterTrifoolUnicornFountain, { CVAR_TRIFOOL_UNICORN_FOUNTAIN });
static RegisterShipInitFunc fountainOverworldInitFunc(RegisterTrifoolFountainOverworld,
                                                      { CVAR_TRIFOOL_FOUNTAIN_OVERWORLD });
static RegisterShipInitFunc dotAdultInitFunc(RegisterTrifoolDoorOfTimeAdult, { CVAR_TRIFOOL_DOT_ADULT });
static RegisterShipInitFunc mimicChestInitFunc(RegisterTrifoolMimicChest, { CVAR_TRIFOOL_MIMIC_CHEST });

// Menu registration moved to menu/TrifoolMenu*.cpp — this file no longer wires widgets.
