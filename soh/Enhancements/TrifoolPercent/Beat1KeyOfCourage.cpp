// TrifoolPercent Beat 1 (Key of Courage), slice 1.1:
//   - Lost Woods secret-path tracker (N -> W -> S -> W) sets BETAKOKIRI_SPAWNED
//   - Beta Kokiri actor registration + spawn in Sacred Forest Meadow once the flag is set
//
// Adapted from Triforce% statics/statics.c (Statics_LostWoods) and actor/BetaKokiri/
// (c) 2019-2022 The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt.
//
// Scope note: the Beta Kokiri actor itself ships with just the body+head combo upstream used
// (object_oE4 skeleton + object_oE9 "mintCap" head + object_oE_anime standing anim). The
// three sibling body slots (oE1s/oE2/oE5) and four sibling head slots (oE6/oE8/oE10/oE11)
// are present in the decomp and extracted by Torch, but are left commented out below until
// isaac asks for the "beta Kokiri rando" flare pass.

#include <array>

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/ActorDB.h"
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

// -----------------------------------------------------------------------------
// Beat 1 save-state flags (session-scoped for now).
// -----------------------------------------------------------------------------
// Beat 1.1 keeps the flag in a module-local static. A follow-up slice will land the
// SaveManager custom section so these persist per-save-file; until then the flag resets
// on game restart and the dev-warps in Setup.cpp still cover testing.
static bool sBetaKokiriSpawned = false;

extern "C" bool TrifoolBeat1_IsBetaKokiriSpawned(void) {
    return sBetaKokiriSpawned;
}

static void SetBetaKokiriSpawned() {
    if (sBetaKokiriSpawned) {
        return;
    }
    sBetaKokiriSpawned = true;
    if (gPlayState != nullptr) {
        Player* player = GET_PLAYER(gPlayState);
        Audio_PlayActorSound2(&player->actor, 0x4802); // the "secret found" cue upstream plays
    }
}

static void ResetBeat1Flags() {
    sBetaKokiriSpawned = false;
}

// -----------------------------------------------------------------------------
// Lost Woods secret-path tracker (Statics_LostWoods adaptation).
// -----------------------------------------------------------------------------
// Three XZ "exit zones" inside Lost Woods, each with 80 unit radius. Walking out of them
// in the order N, W, S, W sets BETAKOKIRI_SPAWNED. Coordinates and ordering come straight
// from upstream statics.c (sWoodsTargetX/Z/Path).

namespace {
constexpr s32 kSceneLostWoods = SCENE_LOST_WOODS; // 0x5B
constexpr s32 kSceneSacredForestMeadow = SCENE_SACRED_FOREST_MEADOW; // 0x56

struct LostWoodsZone { float x; float z; };
constexpr LostWoodsZone kLostWoodsZones[] = {
    { 0.0f,   -530.0f }, // 0: north
    { 793.0f,  546.0f }, // 1: south
    { 269.0f, -795.0f }, // 2: west
};
constexpr s8 kLostWoodsPath[4] = { 0, 2, 1, 2 }; // N, W, S, W
constexpr float kLostWoodsRadius = 80.0f;
} // namespace

static s8 sWoodsState = 0;      // 0 = idle, 1 = in Lost Woods, >=20 = completion sound playing
static s8 sWoodsTarget = -1;    // zone the player is currently inside (-1 = none)
static s8 sWoodsCount = 0;      // how far along kLostWoodsPath we have matched

static s8 FindCurrentZone(float x, float z) {
    for (s32 i = 0; i < 3; ++i) {
        float dx = kLostWoodsZones[i].x - x;
        float dz = kLostWoodsZones[i].z - z;
        if (dx * dx + dz * dz < kLostWoodsRadius * kLostWoodsRadius) {
            return (s8)i;
        }
    }
    return -1;
}

static void TickLostWoodsPath() {
    if (CVarGetInteger(CVAR_TRIFOOL_LOST_WOODS_PATH, 0) == 0 || gPlayState == nullptr) {
        return;
    }

    if (gPlayState->sceneNum != kSceneLostWoods) {
        // Left Lost Woods: if the exit matches the next step on the path, advance; else reset.
        if (sWoodsState == 1 && sWoodsCount < 4) {
            if (sWoodsTarget == kLostWoodsPath[sWoodsCount]) {
                ++sWoodsCount;
                if (sWoodsCount == 4) {
                    SetBetaKokiriSpawned();
                }
            } else {
                sWoodsCount = 0;
            }
            sWoodsState = 0;
        }
        return;
    }

    sWoodsState = 1;
    Player* player = GET_PLAYER(gPlayState);
    sWoodsTarget = FindCurrentZone(player->actor.world.pos.x, player->actor.world.pos.z);
}

// -----------------------------------------------------------------------------
// Beta Kokiri actor: registration + spawn in Sacred Forest Meadow.
// -----------------------------------------------------------------------------

static s16 sBetaKokiriActorId = -1;

// Upstream placed Beta Kokiri in the scene's actor list; we spawn her from code here so the
// ActorDB id can be assigned at runtime. The position is a reasonable default near the Lost
// Woods entrance into Sacred Forest Meadow; tune in Beat 1.3 once we have her dialogue in.
constexpr Vec3f kBetaKokiriSpawnPos = { -15.0f, 0.0f, 1800.0f };
constexpr s16   kBetaKokiriSpawnRotY = 0x0000;

static s16 AddActor(const char* name, const char* desc, size_t instanceSize, ActorFunc init,
                    ActorFunc destroy, ActorFunc update, ActorFunc draw) {
    ActorDBInit entry;
    entry.name = name;
    entry.desc = desc;
    entry.category = ACTORCAT_NPC;
    entry.flags = ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_TARGETABLE;
    entry.objectId = OBJECT_GAMEPLAY_KEEP;
    entry.instanceSize = instanceSize;
    entry.init = init;
    entry.destroy = destroy;
    entry.update = update;
    entry.draw = draw;
    return static_cast<s16>(ActorDB::Instance->AddEntry(entry).entry.id);
}

static void RegisterBetaKokiriActor() {
    if (sBetaKokiriActorId >= 0) {
        return;
    }
    sBetaKokiriActorId =
        AddActor("Tf_Beta_Kokiri", "Beta Kokiri (TrifoolPercent)", sizeof(TfBetaKokiri),
                 TfBetaKokiri_Init, TfBetaKokiri_Destroy, TfBetaKokiri_Update,
                 TfBetaKokiri_Draw);
}

static void SpawnBetaKokiriIfReady() {
    if (CVarGetInteger(CVAR_TRIFOOL_BETA_KOKIRI, 0) == 0 || gPlayState == nullptr) {
        return;
    }
    if (gPlayState->sceneNum != kSceneSacredForestMeadow) {
        return;
    }
    if (!TrifoolBeat1_IsBetaKokiriSpawned()) {
        return;
    }
    RegisterBetaKokiriActor();
    if (sBetaKokiriActorId < 0) {
        return;
    }
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, sBetaKokiriActorId,
                kBetaKokiriSpawnPos.x, kBetaKokiriSpawnPos.y, kBetaKokiriSpawnPos.z,
                0, kBetaKokiriSpawnRotY, 0, 0);
}

// -----------------------------------------------------------------------------
// Dialogue text (0x0B10 only for Beat 1.1).
// -----------------------------------------------------------------------------
// Upstream's full set is 0x0B10-0x0B16; the trade / "thanks" lines land with the Butterfly
// item in Beat 1.3. 0x0B10 is the "I'm looking for a bug" greeting Link gets before any
// trade is possible, which matches today's actor behaviour.

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_COLOR_GREEN "\x05\x42"
#define MSG_COLOR_WHITE "\x05\x40"

static const char kBetaKokiriBugText[] =
    "Hey, hey, " MSG_COLOR_GREEN "Mr. Hero" MSG_COLOR_WHITE "!" MSG_NEWLINE
    "I'm looking for a flying bug..." MSG_NEWLINE
    "A pretty one. Have you seen one?" MSG_END;

static void LoadBeat1Message(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_BETA_KOKIRI, 0) == 0) {
        return;
    }
    if (*textId == 0x0B10) {
        std::string body(kBetaKokiriBugText, sizeof(kBetaKokiriBugText) - 1);
        CustomMessage msg(body, body, body, TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM);
        msg.LoadIntoFont();
        *loadFromMessageTable = false;
    }
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolBeat1() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickLostWoodsPath);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneSpawnActors>(SpawnBetaKokiriIfReady);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(LoadBeat1Message);
    // Clear per-session flags when a save is loaded so switching files does not carry progress.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ResetBeat1Flags(); });
}

static RegisterShipInitFunc beat1InitFunc(RegisterTrifoolBeat1,
                                          { CVAR_TRIFOOL_LOST_WOODS_PATH,
                                            CVAR_TRIFOOL_BETA_KOKIRI });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolBeat1Widgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 1 - Lost Woods path", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_LOST_WOODS_PATH)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Walking N -> W -> S -> W out of Lost Woods arms the Beta Kokiri spawn in Sacred "
            "Forest Meadow, same path trigger as the Triforce% showcase. The 'secret found' "
            "sound plays when the sequence completes."));
    SohGui::mSohMenu->AddWidget(path, "Beat 1 - Beta Kokiri", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_BETA_KOKIRI)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Once the Lost Woods path is armed, spawns the unused beta Kokiri NPC "
            "(object_oE4 skeleton + object_oE9 head + object_oE_anime standing anim, same "
            "combo Triforce% used) in Sacred Forest Meadow. Talking plays the 'I'm looking "
            "for a bug' text (0x0B10). The Butterfly trade lands in Beat 1.3."));
}

static RegisterMenuInitFunc beat1MenuInitFunc(RegisterTrifoolBeat1Widgets);
