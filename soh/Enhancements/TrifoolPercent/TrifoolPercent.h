#ifndef TRIFOOLPERCENT_H
#define TRIFOOLPERCENT_H

// TrifoolPercent: a native Ship of Harkinian rebuild of the Triforce% ending.
// Shared declarations between the C actors and the C++ glue in TrifoolPercent.cpp.

#include <libultraship/libultra.h>
#include "global.h"
#include "align_asset_macro.h"

#define CVAR_TRIFOOL_TRIFORCE_ROOM CVAR_ENHANCEMENT("TrifoolPercent.TriforceRoom")
#define CVAR_TRIFOOL_TEST_ENTRANCE CVAR_ENHANCEMENT("TrifoolPercent.TestEntrance")
#define CVAR_TRIFOOL_UNICORN_FOUNTAIN CVAR_ENHANCEMENT("TrifoolPercent.UnicornFountain")
#define CVAR_TRIFOOL_SETUP_PRESET CVAR_ENHANCEMENT("TrifoolPercent.SetupPreset")
#define CVAR_TRIFOOL_DEV_INPUTS CVAR_ENHANCEMENT("TrifoolPercent.DevInputs")
#define CVAR_TRIFOOL_PAUSE_DEBUG_INV CVAR_ENHANCEMENT("TrifoolPercent.PauseDebugInv")
#define CVAR_TRIFOOL_LOST_WOODS_PATH CVAR_ENHANCEMENT("TrifoolPercent.LostWoodsPath")
#define CVAR_TRIFOOL_BETA_KOKIRI CVAR_ENHANCEMENT("TrifoolPercent.BetaKokiri")
#define CVAR_TRIFOOL_BUTTERFLY CVAR_ENHANCEMENT("TrifoolPercent.Butterfly")
#define CVAR_TRIFOOL_TEST_ROOM CVAR_ENHANCEMENT("TrifoolPercent.TestRoom")
#define CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS CVAR_ENHANCEMENT("TrifoolPercent.TestRoomPrototypeCs")
#define CVAR_TRIFOOL_ONBOARDING CVAR_ENHANCEMENT("TrifoolPercent.Onboarding")
#define CVAR_TRIFOOL_FOUNTAIN_OVERWORLD CVAR_ENHANCEMENT("TrifoolPercent.FountainOverworld")
#define CVAR_TRIFOOL_DOT_ADULT CVAR_ENHANCEMENT("TrifoolPercent.DoorOfTimeAdult")
#define CVAR_TRIFOOL_ENDING CVAR_ENHANCEMENT("TrifoolPercent.Ending")
#define CVAR_TRIFOOL_FOUNTAIN_ENTRY_CS CVAR_ENHANCEMENT("TrifoolPercent.FountainEntryCs")
#define CVAR_TRIFOOL_MIMIC_CHEST CVAR_ENHANCEMENT("TrifoolPercent.MimicChest")
#define CVAR_TRIFOOL_MEDALLION_WATER CVAR_ENHANCEMENT("TrifoolPercent.Medallion.Water")
#define CVAR_TRIFOOL_MEDALLION_LIGHT CVAR_ENHANCEMENT("TrifoolPercent.Medallion.Light")
#define CVAR_TRIFOOL_MEDALLION_FOREST CVAR_ENHANCEMENT("TrifoolPercent.Medallion.Forest")
#define CVAR_TRIFOOL_MEDALLION_FIRE CVAR_ENHANCEMENT("TrifoolPercent.Medallion.Fire")
#define CVAR_TRIFOOL_SONG_ENGINE CVAR_ENHANCEMENT("TrifoolPercent.SongEngine")
#define CVAR_TRIFOOL_LONG_OF_TIME CVAR_ENHANCEMENT("TrifoolPercent.LongOfTime")
#define CVAR_TRIFOOL_RUNNING_MAN  CVAR_ENHANCEMENT("TrifoolPercent.RunningMan")

// Running Man boss dialogue (post-fight NPC form). Vanilla OoT does not use the
// 0x0C30..0x0C34 range -- Triforce% claimed it for the Running Man / Sages' Charm
// handoff. Text copied verbatim from Triforce% assets/text/messages.c.
#define TRIFOOL_TEXT_RUNNING_MAN_C30 0x0C30
#define TRIFOOL_TEXT_RUNNING_MAN_C31 0x0C31
#define TRIFOOL_TEXT_RUNNING_MAN_C32 0x0C32
#define TRIFOOL_TEXT_RUNNING_MAN_C33 0x0C33
#define TRIFOOL_TEXT_RUNNING_MAN_C34 0x0C34
#define CVAR_TRIFOOL_SAGES_CHARM CVAR_ENHANCEMENT("TrifoolPercent.SagesCharm")
#define CVAR_TRIFOOL_MEDALLION_ON_C CVAR_ENHANCEMENT("TrifoolPercent.MedallionOnC")
#define CVAR_TRIFOOL_LONG_OF_TIME_AGE_FLIP CVAR_ENHANCEMENT("TrifoolPercent.LongOfTimeAgeFlip")
#define CVAR_TRIFOOL_SACRED_REALM_CHAIN CVAR_ENHANCEMENT("TrifoolPercent.SacredRealmChain")
#define CVAR_TRIFOOL_STAFF_ROLL_CHAIN CVAR_ENHANCEMENT("TrifoolPercent.StaffRollChain")
#define CVAR_TRIFOOL_RUNNING_MAN_DEBUG_PULSE CVAR_ENHANCEMENT("TrifoolPercent.RunningManDialoguePulse")
#define CVAR_TRIFOOL_SAGES_CHARM_GRANT_PULSE CVAR_ENHANCEMENT("TrifoolPercent.SagesCharmGrantPulse")

// Butterfly rides a slot that vanilla OoT ships but never uses. The gameplay-item id
// 0x68 (object_gi_butterfly, documented on TCRF) has no in-game pickup wired to it, so
// claiming it as the Butterfly displaces nothing. The exchange-item id 0x18 sits in the
// same gap -- the vanilla EXCH_ITEM_* enum stops before it -- so Z-targeting Beta
// Kokiri with the Butterfly bottle maps cleanly to its own trade branch without
// repurposing an existing trade item.
#define TRIFOOL_ITEM_BUTTERFLY 0x68
#define TRIFOOL_EXCH_ITEM_BUTTERFLY 0x18

// Sages' Charm (upstream calls it both "Sages' Charm" and "Oracle Amulet" across the
// Triforce% source and showcase VOD; the in-world text is "Sages' Charm"). Upstream
// repurposed the Stone of Agony GI slot (ITEM_STONE_OF_AGONY = 0x3C) because the Stone
// of Agony is a quest-status item carried off a Rumble Pak check that the showcase ROM
// did not need. We do not want to displace a shipping SoH item, so we claim the next
// unused gameplay-item slot above Butterfly (0x69). TCRF lists 0x66-0x79 as a run of
// quest-status / unused ids, 0x68 is Butterfly, so 0x69 is still a vanilla "no pickup"
// slot and nothing shipped currently reads it. The exported model
// (assets/model/SagesCharm/object_gi_sagescharm.c, display list gGiSagesCharmDL at
// 0x6001128) is already in the repo; the OTR wiring lands when the o2r build adds the
// SagesCharm object, same follow-up as the Butterfly's own 3D asset wiring.
#define TRIFOOL_ITEM_SAGES_CHARM 0x69

// Item-get text id for the Sages' Charm card. Butterfly uses 0x0083; 0x0084 is the next
// free vanilla text id and is loaded through CustomMessage, same pattern as Butterfly.
#define TRIFOOL_TEXT_SAGES_CHARM_GET 0x0084

// Resources shipped in TrifoolPercent.o2r (built by actors/triforce_actor/make_torch_input.py).
#define dgTrifoolTriforcePieceDL "__OTR__objects/object_trifool_triforce_piece/gTrifoolTriforcePieceDL"
static const ALIGN_ASSET(2) char gTrifoolTriforcePieceDL[] = dgTrifoolTriforcePieceDL;

#define dgTrifoolTriforceChestSkel "__OTR__objects/object_trifool_triforce_chest/gTrifoolTriforceChestSkel"
static const ALIGN_ASSET(2) char gTrifoolTriforceChestSkel[] = dgTrifoolTriforceChestSkel;

#define dgTrifoolTriforceChestOpenAnim "__OTR__objects/object_trifool_triforce_chest/gTrifoolTriforceChestOpenAnim"
static const ALIGN_ASSET(2) char gTrifoolTriforceChestOpenAnim[] = dgTrifoolTriforceChestOpenAnim;

#define dgTrifoolTriforceLightSkel "__OTR__objects/object_trifool_triforce_light/gTrifoolTriforceLightSkel"
static const ALIGN_ASSET(2) char gTrifoolTriforceLightSkel[] = dgTrifoolTriforceLightSkel;

#define dgTrifoolTriforceRoomTitleCardTex "__OTR__textures/trifool_place_title_cards/gTrifoolTriforceRoomTitleCardTex"
static const ALIGN_ASSET(2) char gTrifoolTriforceRoomTitleCardTex[] = dgTrifoolTriforceRoomTitleCardTex;

#define dgTrifoolUnicornFountainTitleCardTex "__OTR__textures/trifool_place_title_cards/gTrifoolUnicornFountainTitleCardTex"
static const ALIGN_ASSET(2) char gTrifoolUnicornFountainTitleCardTex[] = dgTrifoolUnicornFountainTitleCardTex;

#define dgTrifoolTestRoomTitleCardTex "__OTR__textures/trifool_place_title_cards/gTrifoolTestRoomTitleCardTex"
static const ALIGN_ASSET(2) char gTrifoolTestRoomTitleCardTex[] = dgTrifoolTestRoomTitleCardTex;

#define dgTrifoolTriforceLightAnim "__OTR__objects/object_trifool_triforce_light/gTrifoolTriforceLightAnim"
static const ALIGN_ASSET(2) char gTrifoolTriforceLightAnim[] = dgTrifoolTriforceLightAnim;

#ifdef __cplusplus
extern "C" {
#endif

// The Triforce Room cutscene from Triforce% (scene/TriforceRoom/TriforceRoom_scene.c), converted to SoH macros.
extern CutsceneData gTrifoolTriforceRoomCs[];
// Frame at which Triforce% handed over to its ending scene. The proof of concept leaves the room here instead.
#define TRIFOOL_TRIFORCE_ROOM_CS_EXIT_FRAME 1885

// Test Room prototype cutscene (TestRoomCs.c): hand-written orbit around the hex hub, three
// textboxes, no actors. Proves the Fast64-free authoring path end-to-end.
extern CutsceneData gTrifoolTestRoomPrototypeCs[];
extern const u16 gTrifoolTestRoomPrototypeCsTextBase;
extern const u8 gTrifoolTestRoomPrototypeCsTextCount;
#define TRIFOOL_TEST_ROOM_PROTOTYPE_CS_EXIT_FRAME 290

// Unicorn Fountain entry cutscene (UnicornFountainCs.c): camera-only arrival pan around the
// fountain centre, 240 frames. The fairy actor holds in state 0 while this plays.
extern CutsceneData gTrifoolUnicornFountainEntryCs[];
#define TRIFOOL_UNICORN_FOUNTAIN_ENTRY_CS_FRAMES 240

// Ported Triforce% cutscenes (PortedCutscenes.c). Dormant data - the arrays are compiled into
// the module so a future hook can trigger any of them via `csCtx.segment` + `cutsceneTrigger`
// without having to re-run Fast64 or copy the vendored source each time. Triggers, text ids
// and custom BGM slots land per-phase in docs/cutscene-plan.md (Phases B-G).
extern CutsceneData gTrifoolNabooruPanAwayCS[];                  // Phase D - gerudoway
extern CutsceneData gTrifoolNabooruSecondHalfCS[];               // Phase D - gerudoway (Long of Time teach)
extern CutsceneData gTrifoolNbColossusCS[];                      // Phase D - spot11 pan to Nabooru
extern CutsceneData gTrifoolspot04cspatch[];                     // Phase D - spot04 CS append (no CS_BEGIN - splice shape)
extern CutsceneData gTrifoolBossRunningManIntroCS[];             // Phase E - Running Man intro
extern CutsceneData gTrifoolBossRunningManOutroCS[];             // Phase E - Running Man outro (Sages' Charm)
extern CutsceneData gTrifoolWarpToSacredRealmCS[];               // Phase B - tokinoma -> Sacred Realm warp
extern CutsceneData gTrifoolHyruleFieldFlyover[];                // Phase G - staff roll flyover 1
extern CutsceneData gTrifoolDeathMountainTrailStaffRollCS[];     // Phase G - staff roll flyover 2
extern CutsceneData gTrifoolGerudoValleyFlyover[];               // Phase G - staff roll flyover 3
extern CutsceneData gTrifoolDekuTreeFlyover[];                   // Phase G - staff roll flyover 4
extern CutsceneData gTrifoolZoraFountainFlyover[];               // Phase G - staff roll flyover 5
extern CutsceneData gTrifoolZoraRiverFlyover[];                  // Phase G - staff roll flyover 6
extern CutsceneData gTrifoolLonLonFlyover[];                     // Phase G - staff roll flyover 7
extern CutsceneData gTrifoolHauntedWastelandFlyover[];           // Phase G - staff roll flyover 8
extern CutsceneData gTrifoolCastleFlyover[];                     // Phase G - staff roll flyover 9

// TriforcePiece params: which piece, plus "the end" mode in bits 2+ (used by the Ending scene, not the room).
typedef enum {
    TRIFOOL_TRIFORCE_POWER = 0,
    TRIFOOL_TRIFORCE_WISDOM = 1,
    TRIFOOL_TRIFORCE_COURAGE = 2,
} TrifoolTriforcePieceType;

typedef struct {
    Actor actor;
    u8 state;
    u8 frame;
    u8 theEndMode;
    LightNode* lightNode;
    LightInfo light;
} TfTriforcePiece;

typedef struct {
    Actor actor;
    SkelAnime skelAnime;
    u8 state;
    u8 frame;
} TfTriforceChest;

typedef struct {
    Actor actor;
    SkelAnime skelAnime;
    u8 state;
    u16 frame;
} TfTriforceLight;

typedef struct {
    Actor actor;
    u8 state;
    u8 frames;
    u8 dialoguePhase; // DIALOGUE_* in z_tf_beta_great_fairy.c
    u8 floatFrames;   // frames spent in state 2 before the first textbox opens
    f32 ratio;
    f32 lastFloatY;
    Vec3f wingtip;
    Vec3f lastWingtip;
} TfBetaGreatFairy;

void TfTriforcePiece_Init(Actor* thisx, PlayState* play);
void TfTriforcePiece_Destroy(Actor* thisx, PlayState* play);
void TfTriforcePiece_Update(Actor* thisx, PlayState* play);
void TfTriforcePiece_Draw(Actor* thisx, PlayState* play);

void TfTriforceChest_Init(Actor* thisx, PlayState* play);
void TfTriforceChest_Destroy(Actor* thisx, PlayState* play);
void TfTriforceChest_Update(Actor* thisx, PlayState* play);
void TfTriforceChest_Draw(Actor* thisx, PlayState* play);

void TfTriforceLight_Init(Actor* thisx, PlayState* play);
void TfTriforceLight_Destroy(Actor* thisx, PlayState* play);
void TfTriforceLight_Update(Actor* thisx, PlayState* play);
void TfTriforceLight_Draw(Actor* thisx, PlayState* play);

void TfBetaGreatFairy_Init(Actor* thisx, PlayState* play);
void TfBetaGreatFairy_Destroy(Actor* thisx, PlayState* play);
void TfBetaGreatFairy_Update(Actor* thisx, PlayState* play);
void TfBetaGreatFairy_Draw(Actor* thisx, PlayState* play);

// Set by TfBetaGreatFairy when her fall animation ends; the Unicorn Fountain glue polls it to fade
// Link back to Kokiri Forest after the visit. Cleared when the fairy actor is re-created on scene load.
bool TrifoolUnicornFountain_IsFairyDone(void);
void TrifoolUnicornFountain_ClearFairyDone(void);

// Beta Kokiri (Beat 1.1). Uses the same body+head combo upstream Triforce% wires
// (object_oE4 skeleton, object_oE9 head DL, object_oE_anime standing anim); the variant
// table is scaffolded in Beat1KeyOfCourage.cpp for later expansion.
typedef struct {
    Actor actor;
    SkelAnime skelAnime;
    ColliderCylinder collider;
    Vec3s jointTable[39];
    Vec3s morphTable[39];
    NpcInteractInfo interactInfo; // head/body tracking state for Npc_TrackPoint (func_80034A14)
    u8 initState;
    u8 eyeTextureIndex;
    u8 blinkTimer;
    s32 animBankIdx;
    s32 bodyBankIdx;
    s32 headBankIdx;
} TfBetaKokiri;

s16 TrifoolBeat1_RegisterBetaKokiri(void);

void TfBetaKokiri_Init(Actor* thisx, PlayState* play);
void TfBetaKokiri_Destroy(Actor* thisx, PlayState* play);
void TfBetaKokiri_Update(Actor* thisx, PlayState* play);
void TfBetaKokiri_Draw(Actor* thisx, PlayState* play);

// MimicChest (Triforce% actor/MimicChest/MimicChest.c). Object data (skeleton + two
// animations) compiles into the SoH module directly from mimic_chest_obj.c and the two
// anim files; no vanilla or Torch object bank is involved.
typedef struct {
    Actor actor;
    SkelAnime skelAnime;
    ColliderCylinder collider;
    Vec3s jointTable[3];
    Vec3s morphTable[3];
} TfMimicChest;

void TfMimicChest_Init(Actor* thisx, PlayState* play);
void TfMimicChest_Destroy(Actor* thisx, PlayState* play);
void TfMimicChest_Update(Actor* thisx, PlayState* play);
void TfMimicChest_Draw(Actor* thisx, PlayState* play);

// Unused-flame billboard built from OBJECT_FIRE (vanilla slot 0x0F, TCRF "Dancing
// Flame"). Spawned by TrifoolPercent.cpp in Kokiri Forest as the test-entrance marker.
typedef struct {
    Actor actor;
    s32 objBankIdx;
    u8 ready;
    u32 frames;
    LightNode* lightNode;
    LightInfo light;
} TfUnusedFlame;

void TfUnusedFlame_Init(Actor* thisx, PlayState* play);
void TfUnusedFlame_Destroy(Actor* thisx, PlayState* play);
void TfUnusedFlame_Update(Actor* thisx, PlayState* play);
void TfUnusedFlame_Draw(Actor* thisx, PlayState* play);

// Sages' Charm session flags. Written from Beat2_0SagesCharm.cpp.
// Read by future slices (medallions-on-C, Fire-Medallion-as-Din's-Fire, magic bar).
bool TrifoolBeat_HasSagesCharm(void);
void TrifoolBeat_GrantSagesCharm(void);

// Progression beat flags for cutscene gating. Written from scene glue (Nabooru, Running Man,
// Chamber of Sages) once each beat is landed; read by the Sacred Realm and Staff Roll chain
// reroutes to decide whether to fire follow-on cutscenes. Session-scoped — reset on new save.
// See docs/cutscene-plan.md §2 for the beats these gate.
bool TrifoolBeat_HasMetNabooru(void);
void TrifoolBeat_GrantMetNabooru(void);
bool TrifoolBeat_HasLearnedLongOfTime(void);
void TrifoolBeat_GrantLearnedLongOfTime(void);
bool TrifoolBeat_HasBeatenRunningMan(void);
void TrifoolBeat_GrantBeatenRunningMan(void);
bool TrifoolBeat_HasVisitedChamberOfSages(void);
void TrifoolBeat_GrantVisitedChamberOfSages(void);

#ifdef __cplusplus
}
#endif

#endif
