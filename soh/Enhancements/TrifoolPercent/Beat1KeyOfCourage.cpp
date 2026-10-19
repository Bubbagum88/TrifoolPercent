// TrifoolPercent Beat 1 (Key of Courage), slices 1.1 and 1.2:
//   - 1.1: Lost Woods secret-path tracker (N -> W -> S -> W) sets BETAKOKIRI_SPAWNED,
//          Beta Kokiri actor registration + spawn in Sacred Forest Meadow once the flag is set.
//   - 1.2: Full Beta Kokiri dialogue tree 0x0B10-0x0B16 wired through CustomMessage, with the
//          trade branch driven by the vanilla exchangeItemId mechanic (no new engine code);
//          the "mystery discovered" sound and the actor spawn are both deferred to the next
//          scene entry after the path completes, instead of firing the moment Link leaves
//          Lost Woods -- matches isaac's call on how the beat should read in-game.
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

// -----------------------------------------------------------------------------
// Beat 1 save-state flags (session-scoped for now).
// -----------------------------------------------------------------------------
// Beat 1.1 keeps the flag in a module-local static. A follow-up slice will land the
// SaveManager custom section so these persist per-save-file; until then the flag resets
// on game restart and the dev-warps in Setup.cpp still cover testing.
static bool sBetaKokiriSpawned = false;
// Deferred "mystery discovered" announcement: the sound and the first spawn both land on the
// NEXT scene entry after the Lost Woods sequence completes, so completing the path mid-step
// in Lost Woods itself is quiet and the player hears the cue the moment the Beta Kokiri
// becomes visible in Sacred Forest Meadow.
static bool sBetaKokiriAnnounced = false;
static bool sBetaKokiriDone = false; // set once Link has traded the Butterfly (Beat 1.3)

extern "C" bool TrifoolBeat1_IsBetaKokiriSpawned(void) {
    return sBetaKokiriSpawned;
}

extern "C" bool TrifoolBeat1_IsBetaKokiriDone(void) {
    return sBetaKokiriDone;
}

extern "C" void TrifoolBeat1_SetBetaKokiriDone(void) {
    sBetaKokiriDone = true;
}

static void SetBetaKokiriSpawned() {
    sBetaKokiriSpawned = true;
}

static void ResetBeat1Flags() {
    sBetaKokiriSpawned = false;
    sBetaKokiriAnnounced = false;
    sBetaKokiriDone = false;
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
    entry.flags = ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_ATTENTION_ENABLED;
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

// Shared with Beat1_0Onboarding.cpp so the kick-off Kokiri near Link's house uses the
// same ActorDB entry (same TfBetaKokiri actor, params=1 variant).
extern "C" s16 TrifoolBeat1_RegisterBetaKokiri(void) {
    RegisterBetaKokiriActor();
    return sBetaKokiriActorId;
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

    // First time the actor actually appears in-game: play the "mystery discovered" cue
    // Triforce% used (0x4802). Any later re-entries into Sacred Forest Meadow are quiet.
    if (!sBetaKokiriAnnounced) {
        sBetaKokiriAnnounced = true;
        Player* player = GET_PLAYER(gPlayState);
        Audio_PlayActorSound2(&player->actor, 0x4802);
    }
}

// -----------------------------------------------------------------------------
// Dialogue text (0x0B10-0x0B16).
// -----------------------------------------------------------------------------
// Full Beta Kokiri dialogue tree, lifted from upstream assets/text/messages.c and wired
// through CustomMessage so SoH's native text box renders every line. Picked by the actor
// using the vanilla exchangeItemId plumbing -- no new engine code, same path the vanilla
// trade NPCs use (Granny, Anju, etc).
//
//   0x0B10  greeting, "looking for bugs?"
//   0x0B11  Link offered a non-bug trade item -> "what is that?"
//   0x0B12  Link offered a (vanilla) bug      -> "crawlies are dumb, I want a flying bug"
//   0x0B13  Link offered the Butterfly        -> accept + hand over Odd Potion (Beat 1.3)
//   0x0B15  tail of the Odd Potion give line
//   0x0B16  after the trade is done
//
// The 0x0B13/0x0B15 lines are present already so Beat 1.3 only has to wire the Butterfly
// exchange item itself; today the actor never offers that branch (sBetaKokiriDone stays
// false until Butterfly lands) and the dialogue tree reads as the pre-trade state.

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_BOX_BREAK "\x04"
#define MSG_QUICKTEXT_ON "\x1B"
#define MSG_QUICKTEXT_OFF "\x1C"

static const char kBeta0B10[] =
    "Oh um. Are you looking for bugs," MSG_NEWLINE
    "too? I love bugs! But I can't" MSG_NEWLINE
    "find any. How's your luck?" MSG_END;

static const char kBeta0B11[] =
    "What is that? I asked you for a" MSG_NEWLINE
    "bug!" MSG_END;

static const char kBeta0B12[] =
    "No, no! These are crawlies!" MSG_NEWLINE
    "Crawlies are dumb. I want a flying" MSG_NEWLINE
    "bug. You know, colorful, with wings." MSG_BOX_BREAK
    "They like flowers, so you might" MSG_NEWLINE
    "find one somewhere people grow lots" MSG_NEWLINE
    "of different kinds of flowers." MSG_END;

static const char kBeta0B13[] =
    "Wow, you actually brought it!" MSG_NEWLINE
    MSG_QUICKTEXT_ON "Thanks!" MSG_QUICKTEXT_OFF MSG_BOX_BREAK
    "You're a nice guy, unlike that Mido..." MSG_NEWLINE
    "He never talks to me. Except one" MSG_NEWLINE
    "time he gave me this, and ordered" MSG_NEWLINE
    "me to keep it safe." MSG_BOX_BREAK
    "But I don't really want it... You can" MSG_NEWLINE
    "have it!" MSG_END;

static const char kBeta0B15[] =
    "I don't think you can use it by" MSG_NEWLINE
    "yourself, though..." MSG_END;

static const char kBeta0B16[] =
    "Thanks for bringing me that bug!" MSG_END;

static void LoadOneMessage(u16 textId, const char* body, size_t bodyLen, bool* loadFromMessageTable) {
    std::string s(body, bodyLen);
    CustomMessage msg(s, s, s, TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE);
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
    (void)textId;
}

static void LoadBeat1Message(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_BETA_KOKIRI, 0) == 0) {
        return;
    }
    switch (*textId) {
        case 0x0B10: LoadOneMessage(*textId, kBeta0B10, sizeof(kBeta0B10) - 1, loadFromMessageTable); break;
        case 0x0B11: LoadOneMessage(*textId, kBeta0B11, sizeof(kBeta0B11) - 1, loadFromMessageTable); break;
        case 0x0B12: LoadOneMessage(*textId, kBeta0B12, sizeof(kBeta0B12) - 1, loadFromMessageTable); break;
        case 0x0B13: LoadOneMessage(*textId, kBeta0B13, sizeof(kBeta0B13) - 1, loadFromMessageTable); break;
        case 0x0B15: LoadOneMessage(*textId, kBeta0B15, sizeof(kBeta0B15) - 1, loadFromMessageTable); break;
        case 0x0B16: LoadOneMessage(*textId, kBeta0B16, sizeof(kBeta0B16) - 1, loadFromMessageTable); break;
        default: break;
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
            "cue is deferred until the next time Link enters Sacred Forest Meadow after the "
            "path finishes, matching the moment the Beta Kokiri first appears."));
    SohGui::mSohMenu->AddWidget(path, "Beat 1 - Beta Kokiri", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_BETA_KOKIRI)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Once the Lost Woods path is armed, spawns the unused beta Kokiri NPC "
            "(object_oE4 skeleton + object_oE9 head + object_oE_anime standing anim, same "
            "combo Triforce% used) in Sacred Forest Meadow. Talking walks the full dialogue "
            "tree (0x0B10-0x0B16) using the vanilla exchangeItemId trade mechanic: show a "
            "non-bug item and she rejects it, show a vanilla bug and she asks for a flying "
            "one. The Butterfly trade and its reward land in Beat 1.3."));
}

static RegisterMenuInitFunc beat1MenuInitFunc(RegisterTrifoolBeat1Widgets);
