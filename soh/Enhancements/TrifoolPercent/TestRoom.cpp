// TrifoolPercent hex Test Room glue.
//
// The scene itself (collision, exit list, floor-patch exit IDs 1..6) comes from
// scenes/TestRoom/export_test_room.py, packed into TrifoolPercent.o2r at the SCENE_SYOTES slot
// (map select "118:Syotes"). This file supplies the dynamic half of the room:
//   - six invisible, Z-targetable "signs" spawned at the inside of each doorway, each with its
//     own text id, so Link can walk up to a doorway and read "Entrance N: <destination>";
//   - custom messages for those six text ids, so the signs read the same way as the rest of
//     TrifoolPercent's dialogue (no new scene text table);
//   - a dev-warp menu entry that drops Link in the room on command, standing in for a proper
//     debug-menu wiring until we ship one.
//
// Doorway destinations (which SCENE_SYOTES's exit list points at) and sign labels are
// deliberately kept side-by-side here so a new entrance is one struct entry plus one re-export
// of the scene.

#include <array>
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

#define CVAR_TEST_ROOM_VALUE CVarGetInteger(CVAR_TRIFOOL_TEST_ROOM, 1)
#define CVAR_TEST_ROOM_PROTOTYPE_CS_VALUE CVarGetInteger(CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS, 0)

// Vanilla OoT scene slot: SCENE_SYOTES (0x68). Nothing in the base game loads it, so the hex
// room displaces nothing.
#define TEST_ROOM_SCENE SCENE_SYOTES

// Doorway layout. The six (x, z) positions mirror the ones export_test_room.py places collision
// patches at: HEX_RADIUS * cos/sin of each side midpoint, scaled from Blender units to OoT units
// (Fast64's default ootBlenderScale of 10 for the hm64 feature set -> 1 Blender unit = 10 OoT
// units -> HEX_RADIUS=40 Blender -> 400 OoT at vertex, ~346 at a side midpoint).
#define HEX_SIDE_MIDPOINT_OOT 346.0f // = HEX_RADIUS * cos(30deg) * 10
#define SIGN_INSET_OOT        140.0f // = SIGN_INSET * 10

// Reserved TrifoolPercent text-id block for the test room signs (0x1600..0x1605).
#define TEST_ROOM_SIGN_TEXT_BASE 0x1600

struct TestRoomDoor {
    const char* label;   // "Kokiri Forest", "Triforce Room", ...
    const char* detail;  // longer description shown on the sign
};

static constexpr std::array<TestRoomDoor, 6> kTestRoomDoors = { {
    { "Kokiri Forest",         "Back to the main game,\nby Link's house." },
    { "Triforce Room",         "The Triforce% Triforce\nRoom (POC)." },
    { "Modified Sacred Realm", "Not built yet. Lands you\nin the Chamber of Sages\nfor now." },
    { "Unicorn Fountain",      "The Triforce% Unicorn\nFountain (POC)." },
    { "(reserved)",             "Open slot for the next\nroom we iterate on." },
    { "(reserved)",             "Open slot for the next\nroom we iterate on." },
} };

static bool IsTestRoom() {
    return gPlayState != nullptr && gPlayState->sceneNum == TEST_ROOM_SCENE;
}

// -----------------------------------------------------------------------------
// Sign actor: invisible, Z-targetable, drives a text box.
// -----------------------------------------------------------------------------

static s16 sTestRoomSignActorId = -1;

typedef struct {
    Actor actor;
    ColliderCylinder collider;
    u8 initState;
} TfTestRoomSign;

static ColliderCylinderInitType1 sSignColliderInit = {
    { COLTYPE_NONE, AT_NONE, AC_NONE, OC1_ON | OC1_TYPE_PLAYER, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0x00000000, 0x00, 0x00 },
      TOUCH_NONE, BUMP_NONE, OCELEM_ON, },
    { 24, 60, 0, { 0, 0, 0 } },
};

extern "C" void TfTestRoomSign_Init(Actor* thisx, PlayState* play) {
    TfTestRoomSign* this_ = (TfTestRoomSign*)thisx;
    this_->initState = 0;
    // params carries the sign slot (0..5); clamp defensively.
    u16 slot = (u16)(thisx->params & 0xFFFF);
    if (slot >= kTestRoomDoors.size()) {
        Actor_Kill(thisx);
        return;
    }
    thisx->textId = TEST_ROOM_SIGN_TEXT_BASE + slot;
    thisx->flags |= ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY;
    thisx->targetMode = 6; // same "friendly NPC" range tier as Kokiri children / shop NPCs
    Collider_InitCylinder(play, &this_->collider);
    Collider_SetCylinderType1(play, &this_->collider, thisx, &sSignColliderInit);
    this_->initState = 1;
}

extern "C" void TfTestRoomSign_Destroy(Actor* thisx, PlayState* play) {
    TfTestRoomSign* this_ = (TfTestRoomSign*)thisx;
    if (this_->initState >= 1) {
        Collider_DestroyCylinder(play, &this_->collider);
    }
}

extern "C" void TfTestRoomSign_Update(Actor* thisx, PlayState* play) {
    TfTestRoomSign* this_ = (TfTestRoomSign*)thisx;
    if (this_->initState != 1) {
        return;
    }
    Collider_UpdateCylinder(thisx, &this_->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this_->collider.base);

    if (Actor_IsTalking(thisx, play)) {
        Player* player = GET_PLAYER(play);
        player->actor.textId = thisx->textId;
        return;
    }
    Actor_RequestToTalk(thisx, play);
}

extern "C" void TfTestRoomSign_Draw(Actor* thisx, PlayState* play) {
    (void)thisx;
    (void)play;
    // Invisible for now; a signpost model lands in a follow-up pass.
}

static s16 AddSignActor() {
    ActorDBInit entry;
    entry.name = "Tf_Test_Room_Sign";
    entry.desc = "TrifoolPercent Test Room sign (invisible, talk-only)";
    entry.category = ACTORCAT_NPC;
    entry.flags = ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_TARGETABLE;
    entry.objectId = OBJECT_GAMEPLAY_KEEP;
    entry.instanceSize = sizeof(TfTestRoomSign);
    entry.init = TfTestRoomSign_Init;
    entry.destroy = TfTestRoomSign_Destroy;
    entry.update = TfTestRoomSign_Update;
    entry.draw = TfTestRoomSign_Draw;
    return static_cast<s16>(ActorDB::Instance->AddEntry(entry).entry.id);
}

// -----------------------------------------------------------------------------
// Spawning signs when the room loads.
// -----------------------------------------------------------------------------
// Signs sit on the inside edge of each doorway, facing the room centre, offset by SIGN_INSET
// along the side's normal. The side normals rotate by 60 degrees per sign (matching the hex
// in export_test_room.py).

static void SpawnTestRoomSigns() {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0) {
        return;
    }
    if (sTestRoomSignActorId < 0) {
        sTestRoomSignActorId = AddSignActor();
    }
    if (sTestRoomSignActorId < 0) {
        return;
    }

    // Debug scenes carry no place name; show the Test Room title card the same way the
    // Triforce Room and Unicorn Fountain do.
    TitleCard_InitPlaceName(gPlayState, &gPlayState->actorCtx.titleCtx, (void*)gTrifoolTestRoomTitleCardTex,
                            160, 120, 144, 24, 20);

    const float distFromCenter = HEX_SIDE_MIDPOINT_OOT - SIGN_INSET_OOT;
    // Six side-midpoints of a flat-top hex at 30, 90, 150, 210, 270, 330 degrees. Yaw faces
    // the room centre (opposite the outward normal). OoT yaws are s16 with 0 = +Z.
    static const s16 kSignYaws[6] = {
        (s16)0xC000, // 30deg outward -> face south-west
        (s16)0x0000, // 90deg outward -> face north
        (s16)0x4000, // 150deg outward -> face east
        (s16)0x4000, // 210deg outward -> face east
        (s16)0x0000, // 270deg outward -> face north
        (s16)0xC000, // 330deg outward -> face south-west
    };
    for (s32 slot = 0; slot < (s32)kTestRoomDoors.size(); ++slot) {
        float angle = 0.5235987755982988f + 1.0471975511965976f * (float)slot; // 30 + 60*slot deg
        float x = cosf(angle) * distFromCenter;
        float z = -sinf(angle) * distFromCenter; // OoT Z axis flipped relative to Blender Y
        Actor_Spawn(&gPlayState->actorCtx, gPlayState, sTestRoomSignActorId,
                    x, 0.0f, z, 0, kSignYaws[slot], 0, slot);
    }
}

// -----------------------------------------------------------------------------
// Custom messages for the six signs.
// -----------------------------------------------------------------------------

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_COLOR_WHITE "\x05\x40"
#define MSG_COLOR_YELLOW "\x05\x46"

static std::string BuildSignMessage(s32 slot) {
    const TestRoomDoor& door = kTestRoomDoors[slot];
    std::string body;
    body += MSG_COLOR_YELLOW "Entrance ";
    body += (char)('1' + slot);
    body += ": ";
    body += MSG_COLOR_WHITE;
    body += door.label;
    body += MSG_NEWLINE;
    body += MSG_NEWLINE;
    body += door.detail;
    body += MSG_END;
    return body;
}

static void LoadTestRoomSignMessage(u16* textId, bool* loadFromMessageTable) {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0) {
        return;
    }
    if (*textId >= TEST_ROOM_SIGN_TEXT_BASE && *textId < TEST_ROOM_SIGN_TEXT_BASE + kTestRoomDoors.size()) {
        s32 slot = (s32)(*textId - TEST_ROOM_SIGN_TEXT_BASE);
        std::string body = BuildSignMessage(slot);
        CustomMessage msg(body, body, body, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE);
        msg.LoadIntoFont();
        *loadFromMessageTable = false;
        return;
    }

    if (*textId >= gTrifoolTestRoomPrototypeCsTextBase &&
        *textId < gTrifoolTestRoomPrototypeCsTextBase + gTrifoolTestRoomPrototypeCsTextCount) {
        s32 slot = (s32)(*textId - gTrifoolTestRoomPrototypeCsTextBase);
        static const char* const kLines[] = {
            MSG_COLOR_YELLOW "Prototype cutscene" MSG_NEWLINE MSG_COLOR_WHITE
                             "running in the Test Room." MSG_END,
            MSG_COLOR_YELLOW "Camera authored" MSG_NEWLINE MSG_COLOR_WHITE
                             "against z64cutscene_commands.h." MSG_END,
            MSG_COLOR_YELLOW "Pipeline check:" MSG_NEWLINE MSG_COLOR_WHITE
                             "scene > actor > cutscene." MSG_END,
        };
        std::string body = kLines[slot];
        CustomMessage msg(body, body, body, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE);
        msg.LoadIntoFont();
        *loadFromMessageTable = false;
        return;
    }
}

// -----------------------------------------------------------------------------
// Prototype cutscene trigger.
// -----------------------------------------------------------------------------
// The cutscene itself (gTrifoolTestRoomPrototypeCs) is a plain CutsceneData[] in TestRoomCs.c -
// authored by hand against z64cutscene_commands.h, no Fast64 export in the loop. Here we only
// decide when to play it: on each scene-spawn of SCENE_SYOTES, when the prototype CVar is on.

static void PlayTestRoomPrototypeCs() {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0 || CVAR_TEST_ROOM_PROTOTYPE_CS_VALUE == 0) {
        return;
    }
    gPlayState->csCtx.segment = gTrifoolTestRoomPrototypeCs;
    gSaveContext.cutsceneTrigger = 1;
}

// -----------------------------------------------------------------------------
// Registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolTestRoom() {
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, SpawnTestRoomSigns);
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, PlayTestRoomPrototypeCs);
    COND_HOOK(OnOpenText, CVAR_TEST_ROOM_VALUE, LoadTestRoomSignMessage);
}

static RegisterShipInitFunc testRoomInitFunc(RegisterTrifoolTestRoom, { CVAR_TRIFOOL_TEST_ROOM });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolTestRoomWidgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Test Room (hex hub)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TEST_ROOM)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "A hexagonal dev hub (in the unused Syotes debug slot, map select \"118:Syotes\") "
            "with six doorways: Kokiri Forest, Triforce Room, Modified Sacred Realm (placeholder), "
            "Unicorn Fountain, and two reserved slots. Each doorway's loading zone is baked into "
            "the scene; the six signs are invisible talk-triggers for now. Needs TrifoolPercent.o2r "
            "in the mods folder for the room itself. Reach it via the Dev inputs' warp cycle "
            "(the \"Test Room (hex hub)\" slot) or the OoT debug map select (\"118:Syotes\")."));
    SohGui::mSohMenu->AddWidget(path, "Test Room prototype cutscene", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Plays a 300-frame hand-written cutscene (orbit around the hex hub, three textboxes) "
            "every time you enter the Test Room. Proves the authoring + trigger pipeline from "
            "docs/cutscene-plan.md end-to-end with no Fast64 export. Leave off for ordinary "
            "test-room navigation; turn it on to see or iterate on the cutscene."));
}

static RegisterMenuInitFunc testRoomMenuInitFunc(RegisterTrifoolTestRoomWidgets);
