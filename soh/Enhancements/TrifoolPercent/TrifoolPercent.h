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
#define CVAR_TRIFOOL_ONBOARDING CVAR_ENHANCEMENT("TrifoolPercent.Onboarding")

// Butterfly rides a slot that vanilla OoT ships but never uses. The gameplay-item id
// 0x68 (object_gi_butterfly, documented on TCRF) has no in-game pickup wired to it, so
// claiming it as the Butterfly displaces nothing. The exchange-item id 0x18 sits in the
// same gap -- the vanilla EXCH_ITEM_* enum stops before it -- so Z-targeting Beta
// Kokiri with the Butterfly bottle maps cleanly to its own trade branch without
// repurposing an existing trade item.
#define TRIFOOL_ITEM_BUTTERFLY 0x68
#define TRIFOOL_EXCH_ITEM_BUTTERFLY 0x18

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

void TfBetaKokiri_Init(Actor* thisx, PlayState* play);
void TfBetaKokiri_Destroy(Actor* thisx, PlayState* play);
void TfBetaKokiri_Update(Actor* thisx, PlayState* play);
void TfBetaKokiri_Draw(Actor* thisx, PlayState* play);

#ifdef __cplusplus
}
#endif

#endif
