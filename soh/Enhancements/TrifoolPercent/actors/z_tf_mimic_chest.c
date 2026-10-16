/*
 * TfMimicChest: a chest that lurches toward Link when he gets close.
 *
 * Ported from Triforce% actor/MimicChest/MimicChest.c (c) 2019-2022 The Triforce% Team,
 * used under upstream/TRIFORCE-PERCENT-LICENSE.txt (condition 1: Credits).
 *
 * TrifoolPercent-side differences vs. upstream:
 *   - PlayState / GET_PLAYER everywhere instead of GlobalContext / PLAYER;
 *   - the inline `init_vars` ActorProfile is replaced by ActorDB registration from
 *     TrifoolPercent.cpp (so the actor id is assigned at runtime, not hijacked);
 *   - the object data is compiled directly into the SoH module (see
 *     mimic_chest_obj.c + mimic_chest_*_anim.c) rather than loaded through a vanilla
 *     object bank, since OBJECT_MIMIC_CHEST is not a vanilla slot;
 *   - `func_80093D18` is replaced with the equivalent `Gfx_SetupDL_25_Opa` setup used
 *     by SoH's own enemy actors.
 */

#include "../TrifoolPercent.h"
#include "mimic_chest_obj.h"

#define TF_MC_SHADOW_SIZE 80.0f
#define TF_MC_LURCH_SPEED 1.0f
#define TF_MC_WAKE_DIST   100.0f
#define TF_MC_SLEEP_DIST  300.0f

static ColliderCylinderInit sCylinderInit = {
    { COLTYPE_HIT5, AT_ON | AT_TYPE_ENEMY, AC_ON | AC_TYPE_PLAYER, OC1_ON | OC1_TYPE_ALL,
      OC1_TYPE_1, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK1,
      { 0x00000000, 0x00, 0x00 }, { 0xFFCFFFFF, 0x10, 0x00 },
      TOUCH_ON, BUMP_ON, OCELEM_ON, },
    { 30, 60, 0, { 0, 0, 0 } },
};

static void TfMimicChest_UpdateCommon(TfMimicChest* this, PlayState* play) {
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f, UPDBGCHECKINFO_FLAG_0 |
                                                                     UPDBGCHECKINFO_FLAG_2);
    SkelAnime_Update(&this->skelAnime);
}

static void TfMimicChest_UpdateAttack(Actor* thisx, PlayState* play);

static void TfMimicChest_UpdateWait(Actor* thisx, PlayState* play) {
    TfMimicChest* this = (TfMimicChest*)thisx;
    TfMimicChest_UpdateCommon(this, play);
    if (this->actor.xzDistToPlayer < TF_MC_WAKE_DIST) {
        Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_ENEMY);
        this->actor.flags |= ACTOR_FLAG_TARGETABLE;
        Animation_Change(&this->skelAnime, &MimicChestObjAttackAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&MimicChestObjAttackAnim), ANIMMODE_LOOP, 6.0f);
        this->actor.update = TfMimicChest_UpdateAttack;
    }
}

static void TfMimicChest_UpdateAttack(Actor* thisx, PlayState* play) {
    TfMimicChest* this = (TfMimicChest*)thisx;

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 500, 1);
    f32 sx = Math_SinS(this->actor.shape.rot.y);
    f32 sz = Math_CosS(this->actor.shape.rot.y);
    this->actor.velocity.x = sx * TF_MC_LURCH_SPEED;
    this->actor.velocity.y = CLAMP_MIN(this->actor.gravity, this->actor.minVelocityY);
    this->actor.velocity.z = sz * TF_MC_LURCH_SPEED;
    f32 rate = R_UPDATE_RATE * 0.5f;
    this->actor.world.pos.x += (this->actor.velocity.x * rate) + this->actor.colChkInfo.displacement.x;
    this->actor.world.pos.y += (this->actor.velocity.y * rate) + this->actor.colChkInfo.displacement.y;
    this->actor.world.pos.z += (this->actor.velocity.z * rate) + this->actor.colChkInfo.displacement.z;
    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += 30.0f;
    this->actor.focus.rot = this->actor.world.rot;

    TfMimicChest_UpdateCommon(this, play);

    if (this->actor.xzDistToPlayer > TF_MC_SLEEP_DIST) {
        Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_PROP);
        this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
        Animation_Change(&this->skelAnime, &MimicChestObjIdleAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&MimicChestObjIdleAnim), ANIMMODE_LOOP, 12.0f);
        this->actor.update = TfMimicChest_UpdateWait;
    }
}

void TfMimicChest_Init(Actor* thisx, PlayState* play) {
    TfMimicChest* this = (TfMimicChest*)thisx;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, TF_MC_SHADOW_SIZE);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    SkelAnime_InitFlex(play, &this->skelAnime, &MimicChestObj, &MimicChestObjIdleAnim,
                       this->jointTable, this->morphTable, MIMICCHESTOBJ_NUM_LIMBS);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.gravity = -8.0f;
    this->actor.colChkInfo.mass = 0xFE;
    this->actor.colChkInfo.health = 0xB0;
    this->actor.update = TfMimicChest_UpdateWait;
}

void TfMimicChest_Destroy(Actor* thisx, PlayState* play) {
    TfMimicChest* this = (TfMimicChest*)thisx;
    Collider_DestroyCylinder(play, &this->collider);
    SkelAnime_Free(&this->skelAnime, play);
}

void TfMimicChest_Update(Actor* thisx, PlayState* play) {
    // ActorDB registers this symbol as the actor's update function. Init immediately
    // overwrites actor.update with the state-specific variant (Wait first), so from the
    // next tick on this one isn't called. The forward exists so the ActorDB entry can
    // reference a stable symbol and the state machine stays in private helpers.
    TfMimicChest_UpdateWait(thisx, play);
}

void TfMimicChest_Draw(Actor* thisx, PlayState* play) {
    TfMimicChest* this = (TfMimicChest*)thisx;
    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL_25Opa(play->state.gfxCtx);
    POLY_OPA_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                                       this->skelAnime.dListCount, NULL, NULL, &this->actor,
                                       POLY_OPA_DISP);
    CLOSE_DISPS(play->state.gfxCtx);
}
