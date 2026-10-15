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
#define CVAR_TRIFOOL_FOUNTAIN_ENTRANCE CVAR_ENHANCEMENT("TrifoolPercent.FountainEntrance")

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

#ifdef __cplusplus
}
#endif

#endif
