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

// Reserved TrifoolPercent text-id blocks for the test room signs:
//   0x1600..0x1605 - six doorway signs (destinations the hex's exit list points at).
//   0x1610..0x1612 - the prototype cutscene's three textboxes (TestRoomCs.c).
//   0x1620..0x162D - beta/unused-content gallery plaques scattered around the room interior
//                    (TCRF-sourced descriptions + commemorative plaques for TrifoolPercent assets).
#define TEST_ROOM_SIGN_TEXT_BASE 0x1600
#define TEST_ROOM_EXHIBIT_TEXT_BASE 0x1620

struct TestRoomSign {
    const char* label;   // "Kokiri Forest", "En_Bird (object_bird)", ...
    const char* detail;  // longer description shown on the sign
};

static constexpr std::array<TestRoomSign, 6> kTestRoomDoors = { {
    { "Kokiri Forest",         "Back to the main game,\nby Link's house." },
    { "Triforce Room",         "The Triforce% Triforce\nRoom (POC)." },
    { "Modified Sacred Realm", "Chamber of the Sages, re-\ndressed with beta birds,\nvases, chicks and dancing\nflames. (Static pass, no\nanimation yet.)" },
    { "Unicorn Fountain",      "The Triforce% Unicorn\nFountain (POC)." },
    { "(reserved)",             "Open slot for the next\nroom we iterate on." },
    { "(reserved)",             "Open slot for the next\nroom we iterate on." },
} };

// Beta/unused-content gallery. Each entry is a plaque explaining one OoT beta or unused asset
// (addresses and descriptions sourced from TCRF: "Unused Actors & Objects" and "Unused Content in
// Used Actors & Objects"), plus a few commemorative plaques for the Triforce% assets we've
// already landed on the TrifoolPercent side. Line breaks are hand-wrapped for OoT's narrow textbox.
static constexpr std::array<TestRoomSign, 14> kTestRoomExhibits = { {
    { "En_Part  (0x0007)",
      "Beta disembodied body-part\nactor. Spawns a floating head\nor hand with the vanilla\nStalfos skeleton." },
    { "En_Scene_Change  (0x0024)",
      "Unused trigger that was\nmeant to swap scene headers\nmid-room. Nothing in the\nretail game spawns it." },
    { "En_OE2  (0x004F)",
      "Unused child NPC built from\nobject_oE2. Early sibling of\nthe oE4 Kokiri we repurpose\nfor Beta Kokiri." },
    { "En_Horse_Zelda  (0x005B)",
      "Unused mounted Zelda horse.\nZelda rides Impa's horse in\nthe final escape cutscene\ninstead of her own." },
    { "En_Bird  (0x0072, object_bird)",
      "Unused blue swallow. Was\nmeant to flock around Hyrule\nField; the chirping SFX\nstill ship in the ROM." },
    { "En_Lightbox  (0x007C)",
      "Unused glowing crate actor.\nLikely a scrapped Shadow\nTemple light-source puzzle\nprop." },
    { "En_Pu_box  (0x007D)",
      "Unused pushable box. Beta\nice-cavern or Spirit Temple\npush-block, pre-dating the\nfinal Obj_Oshihiki." },
    { "En_Vase  (0x0082)",
      "Unused Hyrule Castle vase.\nBeta castle courtyard decor;\nreplaced by the vanilla\nbreakable pots." },
    { "En_Nwc  (0x0135)",
      "Unused chick. Lon Lon Ranch\nconcepts had a baby-cucco\npet; its model and chirp\nremain in the ROM." },
    { "En_Clear_Tag  (0x013B)",
      "Beta silver sparkle. Death\nMountain / Spirit Temple test\neffect, superseded by the\nfinal Clear_Tag_Switch." },
    { "Item_Inbox  (0x0137)",
      "Unused chest-like container.\nEarly treasure-pickup actor\nreplaced by En_Box before\nrelease." },
    { "Obj_Elevator  (0x012B)",
      "Unused beta elevator. Lift\nactor from the pre-release\nJabu-Jabu / Shadow Temple\nprototypes." },
    { "object_gi_butterfly  (0x0175)",
      "Vanilla OoT ships this gift\nitem with no in-game pickup.\nTrifoolPercent claims the\nslot for its Butterfly bottle." },
    { "object_fire  (unused dancing flame)",
      "Vanilla OoT slot 0x000F has\na 'Dancing Flame' DL with no\nspawner. We use it for the\ntest-entrance marker." },
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
    // params carries the full text id (0x1600+door or 0x1620+exhibit); validate before trusting it.
    u16 textId = (u16)(thisx->params & 0xFFFF);
    bool isDoor = textId >= TEST_ROOM_SIGN_TEXT_BASE &&
                  textId < TEST_ROOM_SIGN_TEXT_BASE + kTestRoomDoors.size();
    bool isExhibit = textId >= TEST_ROOM_EXHIBIT_TEXT_BASE &&
                     textId < TEST_ROOM_EXHIBIT_TEXT_BASE + kTestRoomExhibits.size();
    if (!isDoor && !isExhibit) {
        Actor_Kill(thisx);
        return;
    }
    thisx->textId = textId;
    thisx->flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY;
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

    if (Actor_ProcessTalkRequest(thisx, play)) {
        Player* player = GET_PLAYER(play);
        player->actor.textId = thisx->textId;
        return;
    }
    Actor_OfferTalkNearColChkInfoCylinder(thisx, play);
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
    entry.flags = ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_ATTENTION_ENABLED;
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
                    x, 0.0f, z, 0, kSignYaws[slot], 0, TEST_ROOM_SIGN_TEXT_BASE + slot);
    }
}

// Spawn the beta-content gallery plaques on an inner ring, well clear of the doorway signs and
// the room centre. Each plaque faces the centre so Link can walk the ring and read them in order.
static void SpawnTestRoomExhibits() {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0) {
        return;
    }
    if (sTestRoomSignActorId < 0) {
        sTestRoomSignActorId = AddSignActor();
    }
    if (sTestRoomSignActorId < 0) {
        return;
    }
    constexpr float kExhibitRingRadius = 220.0f;         // OoT units from room centre
    constexpr float kTwoPi = 6.2831853071795864f;
    constexpr s32 count = (s32)kTestRoomExhibits.size(); // 14 plaques
    for (s32 slot = 0; slot < count; ++slot) {
        float angle = kTwoPi * (float)slot / (float)count;
        float x = cosf(angle) * kExhibitRingRadius;
        float z = -sinf(angle) * kExhibitRingRadius;
        // Yaw: face the centre. In OoT yaw-s16, Math_Vec3f_Yaw(from->to) is +Z basis; here the sign
        // sits at (x, z) and should look at (0, 0), so the outward yaw is atan2f(x, z) in OoT units.
        float outwardAngle = atan2f(x, z); // radians, OoT yaw = radians * 0x8000 / M_PI
        s16 outwardYaw = (s16)(outwardAngle * (32768.0f / 3.14159265358979f));
        s16 inwardYaw = (s16)(outwardYaw + (s16)0x8000);
        Actor_Spawn(&gPlayState->actorCtx, gPlayState, sTestRoomSignActorId,
                    x, 0.0f, z, 0, inwardYaw, 0, TEST_ROOM_EXHIBIT_TEXT_BASE + slot);
    }
}

// Dev-convenience flame at the Unicorn Fountain doorway (sign slot 3). The hex scene's own exit
// list already routes Link to the fountain when he walks through the doorway, so this flame is a
// visual marker only, not a secondary warp. It replaces the old Lost Woods flame that the
// FountainOverworld toggle used to spawn; the canonical overworld entrance is now at the bottom
// of Zora's Domain. Slot 3 angle: 30 + 60*3 = 210 degrees (see SpawnTestRoomSigns).
static void SpawnTestRoomFountainFlame() {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0) {
        return;
    }
    constexpr float kFountainSlotAngle = 3.6651914291880923f; // 30 + 60*3 deg in radians
    constexpr float kFlameDistFromCenter = 240.0f; // inboard of SIGN_INSET so the flame leads to the door
    float x = cosf(kFountainSlotAngle) * kFlameDistFromCenter;
    float z = -sinf(kFountainSlotAngle) * kFlameDistFromCenter;
    TrifoolSpawnUnusedFlame(x, 0.0f, z);
}

// Live beta-actor exhibits: spawn the three vanilla beta actors whose object slots we added to
// the room's object list in export_test_room.py (obj_bird, obj_vase, obj_nwc). They stand on a
// small triangle around the room centre so Link can walk up to each and see the actual actor.
// Keeping this to object-safe actors is deliberate - other beta actors (En_Horse_Zelda, Item_Inbox,
// etc.) need object slots that would push the room's object list closer to the cap or depend on
// gameplay state we don't set up here; those stay as plaque-only entries for now.
static void SpawnTestRoomLiveExhibits() {
    if (!IsTestRoom() || CVAR_TEST_ROOM_VALUE == 0) {
        return;
    }
    constexpr float kRadius = 90.0f;
    struct LiveExhibit {
        s16 actorId;
        s16 params;
    };
    static const LiveExhibit kExhibits[3] = {
        { ACTOR_EN_BIRD, 0x0000 },  // obj_bird: swallow that normally circles overhead
        { ACTOR_EN_VASE, 0x0000 },  // obj_vase: beta Hyrule Castle orange pot
        { ACTOR_EN_NWC,  0x0000 },  // obj_nwc:  cucco chick, unused Lon Lon pet
    };
    for (s32 i = 0; i < 3; ++i) {
        float angle = 2.0943951023931953f * (float)i; // 0, 120, 240 deg
        float x = cosf(angle) * kRadius;
        float z = -sinf(angle) * kRadius;
        Actor_Spawn(&gPlayState->actorCtx, gPlayState, kExhibits[i].actorId,
                    x, 0.0f, z, 0, 0, 0, kExhibits[i].params);
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
    const TestRoomSign& door = kTestRoomDoors[slot];
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

static std::string BuildExhibitMessage(s32 slot) {
    const TestRoomSign& ex = kTestRoomExhibits[slot];
    std::string body;
    body += MSG_COLOR_YELLOW;
    body += ex.label;
    body += MSG_NEWLINE;
    body += MSG_NEWLINE;
    body += MSG_COLOR_WHITE;
    body += ex.detail;
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

    if (*textId >= TEST_ROOM_EXHIBIT_TEXT_BASE &&
        *textId < TEST_ROOM_EXHIBIT_TEXT_BASE + kTestRoomExhibits.size()) {
        s32 slot = (s32)(*textId - TEST_ROOM_EXHIBIT_TEXT_BASE);
        std::string body = BuildExhibitMessage(slot);
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
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, SpawnTestRoomExhibits);
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, SpawnTestRoomLiveExhibits);
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, SpawnTestRoomFountainFlame);
    COND_HOOK(OnSceneSpawnActors, CVAR_TEST_ROOM_VALUE, PlayTestRoomPrototypeCs);
    COND_HOOK(OnOpenText, CVAR_TEST_ROOM_VALUE, LoadTestRoomSignMessage);
}

static RegisterShipInitFunc testRoomInitFunc(RegisterTrifoolTestRoom, { CVAR_TRIFOOL_TEST_ROOM });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterTestRoomSceneWidget() {
    WidgetPath path = { "TrifoolPercent", "Scenes & Rooms", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Test Room (hex hub)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TEST_ROOM)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "A hexagonal dev hub (in the unused Syotes debug slot, map select \"118:Syotes\") "
            "with six doorways: Kokiri Forest, Triforce Room, Modified Sacred Realm (placeholder), "
            "Unicorn Fountain, and two reserved slots. Each doorway's loading zone is baked into "
            "the scene; the six signs are invisible talk-triggers for now. An inner ring of 14 "
            "exhibit plaques describes beta/unused OoT actors and the TrifoolPercent assets we've "
            "landed (TCRF-sourced), and three live beta actors (En_Bird, En_Vase, En_Nwc) stand "
            "near the room centre. Needs TrifoolPercent.o2r "
            "in the mods folder for the room itself. Reach it via the Dev inputs' warp cycle "
            "(the \"Test Room (hex hub)\" slot) or the OoT debug map select (\"118:Syotes\")."));
}

void TrifoolMenu::RegisterTestRoomPrototypeCutsceneWidget() {
    WidgetPath path = { "TrifoolPercent", "Cutscenes", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Test Room prototype cutscene", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Plays a 300-frame hand-written cutscene (orbit around the hex hub, three textboxes) "
            "every time you enter the Test Room. Proves the authoring + trigger pipeline from "
            "docs/cutscene-plan.md end-to-end with no Fast64 export. Leave off for ordinary "
            "test-room navigation; turn it on to see or iterate on the cutscene."));
}
