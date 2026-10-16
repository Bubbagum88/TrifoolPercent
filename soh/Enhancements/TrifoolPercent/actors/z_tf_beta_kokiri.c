/*
 * TfBetaKokiri: the unused beta Kokiri child, standing in Sacred Forest Meadow after Link walks
 * the Lost Woods secret path (N -> W -> S -> W). Beat 1.1 ships the actor with just the body +
 * head combo upstream used; the full variant table is scaffolded for later expansion.
 *
 * Ported from Triforce% actor/BetaKokiri/betakokiri.c (c) 2019-2022 The Triforce% Team, used
 * under upstream/TRIFORCE-PERCENT-LICENSE.txt (condition 1: Credits). Object slot facts come
 * from zeldaret/oot's assets/xml/objects/object_oE4.xml, object_oE9.xml, object_oE_anime.xml.
 *
 * TrifoolPercent-side differences vs. upstream:
 *   - PlayState / GET_PLAYER everywhere instead of GlobalContext / PLAYER;
 *   - the beta-head-turn-toward-player (func_80034A14 / Npc_TrackPoint) is restored, mode 1,
 *     which is the vanilla "player auto-turn" preset: head leads, body follows naturally;
 *   - targetMode is set to the "friendly NPC" range (same tier Kokiri children / shop NPCs
 *     use) and ACTOR_FLAG_FRIENDLY is on so Z-target reads her as an NPC, not a hostile;
 *   - the trade branch uses vanilla exchangeItemId with EXCH_ITEM_BUG covered and
 *     EXCH_ITEM_BUTTERFLY left as a Beat 1.3 hook -- today Butterfly never arrives, so
 *     Link ends up on the "crawlies are dumb" / "what is that?" paths or the greeting,
 *     exactly like upstream pre-trade;
 *   - the SPAWNED and DONE flags live behind TrifoolBeat1_* accessors rather than borrowed
 *     infTable / itemGetInf bits, so we never clobber vanilla save state;
 *   - the "give Odd Potion" / Bottle-swap branch stays scaffolded but inert until the
 *     Butterfly item (Beat 1.3) can hand Link EXCH_ITEM_BUTTERFLY in the first place.
 */

#include "../TrifoolPercent.h"

// Vanilla OoT object symbols published by Torch when it extracts the beta object slots from
// the user's ROM. Offsets and names come straight from zeldaret/oot's XML descriptors.
extern SkeletonHeader object_oE4_Skel_000260;
extern Gfx object_oE9_DL_000800[];
extern u8 object_oE9_Tex_000E90[]; // eyes open   (ci8 32x32, TLUT 0xC90)
extern u8 object_oE9_Tex_001290[]; // eyes half   (ci8 32x32)
extern u8 object_oE9_Tex_001690[]; // eyes closed (ci8 32x32)
extern AnimationHeader object_oE_anime_Anim_00001034; // kokiriStanding (upstream's choice)

// Object slots. These are vanilla OoT slot IDs; SoH's object loader indexes them the same way.
#define TF_BK_OBJ_ANIM 0x34 // object_oE_anime
#define TF_BK_OBJ_BODY 0x4C // object_oE4
#define TF_BK_OBJ_HEAD 0x52 // object_oE9

#define TF_BK_LIMB_HEAD 23
#define TF_BK_NUM_LIMBS 39
#define TF_BK_SHADOW_SIZE 25.0f

// Head-tracking tuning. Mode 1 is vanilla "player auto-turn": the engine itself lets the
// head lead and the body follow at a feel the rest of the game is tuned for.
#define TF_BK_HEAD_FOLLOW_PRESET 0xC  // same value upstream Triforce% passed (kREG(17)+0xC)
#define TF_BK_HEAD_FOLLOW_MODE 1      // vanilla NPC_TRACKING_PLAYER_AUTO_TURN
#define TF_BK_PLAYER_EYE_HEIGHT 9.0f

bool TrifoolBeat1_IsBetaKokiriSpawned(void); // defined in Beat1KeyOfCourage.cpp
bool TrifoolBeat1_IsBetaKokiriDone(void);
void TrifoolBeat1_SetBetaKokiriDone(void);

static ColliderCylinderInitType1 sCylinderInit = {
    { COLTYPE_HIT0, AT_NONE, AC_NONE, OC1_ON | OC1_TYPE_PLAYER, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0x00000000, 0x00, 0x00 },
      TOUCH_NONE, BUMP_NONE, OCELEM_ON, },
    { 20, 80, 0, { 0, 0, 0 } },
};

static void* sEyeTextures[3] = {
    (void*)object_oE9_Tex_000E90,
    (void*)object_oE9_Tex_001290,
    (void*)object_oE9_Tex_001690,
};

static void TfBetaKokiri_SetBank(TfBetaKokiri* this, PlayState* play, s32 bank, s32 alsoRCP) {
    void* segment = play->objectCtx.status[bank].segment;
    gSegments[6] = VIRTUAL_TO_PHYSICAL(segment);
    if (alsoRCP) {
        OPEN_DISPS(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x06, segment);
        CLOSE_DISPS(play->state.gfxCtx);
    }
}

static void TfBetaKokiri_UpdateEyes(TfBetaKokiri* this) {
    if (this->blinkTimer == 0) {
        this->blinkTimer = Rand_S16Offset(60, 60);
    } else {
        --this->blinkTimer;
    }
    this->eyeTextureIndex = (this->blinkTimer < 3) ? this->blinkTimer : 0;
}

static s32 TfBetaKokiri_ObjectsLoaded(TfBetaKokiri* this, PlayState* play) {
    return Object_IsLoaded(&play->objectCtx, this->animBankIdx) &&
           Object_IsLoaded(&play->objectCtx, this->bodyBankIdx) &&
           Object_IsLoaded(&play->objectCtx, this->headBankIdx);
}

static void TfBetaKokiri_UpdateWaitTalk(Actor* thisx, PlayState* play);

void TfBetaKokiri_Init(Actor* thisx, PlayState* play) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;

    if (!TrifoolBeat1_IsBetaKokiriSpawned()) {
        Actor_Kill(&this->actor);
        return;
    }

    this->initState = 0;
    this->blinkTimer = 0;
    this->eyeTextureIndex = 0;
    this->actor.textId = 0x0B10;

    // NPC lock-on, not enemy. ACTOR_FLAG_FRIENDLY makes Z-target treat her as a talkable
    // NPC (green reticle, no attack prompt); ACTOR_FLAG_TARGETABLE was already set at
    // ActorDB registration time. targetMode 6 is the vanilla "friendly NPC" range tier
    // (what shopkeepers and Kokiri children use).
    this->actor.flags |= ACTOR_FLAG_FRIENDLY;
    this->actor.targetMode = 6;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, TF_BK_SHADOW_SIZE);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);

    this->animBankIdx = Object_GetIndex(&play->objectCtx, TF_BK_OBJ_ANIM);
    this->bodyBankIdx = Object_GetIndex(&play->objectCtx, TF_BK_OBJ_BODY);
    this->headBankIdx = Object_GetIndex(&play->objectCtx, TF_BK_OBJ_HEAD);
    if (this->animBankIdx < 0 || this->bodyBankIdx < 0 || this->headBankIdx < 0) {
        Actor_Kill(&this->actor);
        return;
    }
    this->initState = 1;
    this->actor.update = TfBetaKokiri_UpdateWaitTalk;
}

void TfBetaKokiri_Destroy(Actor* thisx, PlayState* play) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;
    if (this->initState >= 1) {
        Collider_DestroyCylinder(play, &this->collider);
    }
    if (this->initState == 2) {
        SkelAnime_Free(&this->skelAnime, play);
    }
}

static void TfBetaKokiri_UpdateTracking(TfBetaKokiri* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    // Vanilla NPC tracking: mode 1 is the engine's own "head leads, body follows" preset
    // (same one Kokiri children, shop NPCs, Nabooru, etc. use). Feed it the player's
    // eyeline and let it tune the feel; no custom deadband here.
    this->interactInfo.trackPos = player->actor.world.pos;
    this->interactInfo.yOffset = TF_BK_PLAYER_EYE_HEIGHT;
    func_80034A14(&this->actor, &this->interactInfo, TF_BK_HEAD_FOLLOW_PRESET,
                  TF_BK_HEAD_FOLLOW_MODE);
}

static s32 TfBetaKokiri_UpdateCommon(TfBetaKokiri* this, PlayState* play) {
    if (!TfBetaKokiri_ObjectsLoaded(this, play)) {
        return -1;
    }
    if (this->initState == 1) {
        this->initState = 2;
        TfBetaKokiri_SetBank(this, play, this->bodyBankIdx, 0);
        SkelAnime_Init(play, &this->skelAnime, &object_oE4_Skel_000260, NULL,
                       this->jointTable, this->morphTable, TF_BK_NUM_LIMBS);
        TfBetaKokiri_SetBank(this, play, this->animBankIdx, 0);
        Animation_Change(&this->skelAnime, &object_oE_anime_Anim_00001034, 1.0f, 0.0f,
                         Animation_GetLastFrame(&object_oE_anime_Anim_00001034),
                         ANIMMODE_LOOP, 0.0f);
    }

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f, 5);
    TfBetaKokiri_UpdateEyes(this);
    TfBetaKokiri_UpdateTracking(this, play);
    return SkelAnime_Update(&this->skelAnime);
}

static void TfBetaKokiri_UpdateWaitTalk(Actor* thisx, PlayState* play) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;
    Player* player = GET_PLAYER(play);

    if (TfBetaKokiri_UpdateCommon(this, play) < 0) {
        return;
    }

    if (Actor_IsTalking(&this->actor, play)) {
        s8 exch = player->exchangeItemId;
        if (TrifoolBeat1_IsBetaKokiriDone()) {
            // Beat 1.3 will arm this once the Butterfly trade is accepted; until then it
            // never fires, matching upstream's pre-trade behaviour.
            this->actor.textId = 0x0B16;
        } else if (exch == EXCH_ITEM_BUG) {
            // Vanilla "Bug in a Bottle" (child trade slot). She rejects it as a crawly.
            this->actor.textId = 0x0B12;
            func_80078884(NA_SE_SY_ERROR);
#ifdef EXCH_ITEM_BUTTERFLY
        } else if (exch == EXCH_ITEM_BUTTERFLY) {
            // Reserved for Beat 1.3: swap the Butterfly bottle for an Odd Potion and advance.
            // The EXCH_ITEM_BUTTERFLY enum lands with the Butterfly item itself, so this
            // branch stays guarded until then.
            this->actor.textId = 0x0B13;
            func_80078884(NA_SE_SY_TRE_BOX_APPEAR);
            Player_UpdateBottleHeld(play, player, ITEM_BOTTLE, PLAYER_AP_BOTTLE);
            TrifoolBeat1_SetBetaKokiriDone();
#endif
        } else if (exch != EXCH_ITEM_NONE) {
            // Any other trade item on offer: Zelda's Letter, egg, chicken, etc. She rejects.
            this->actor.textId = 0x0B11;
        } else {
            // Default greeting: she's looking for a flying bug.
            this->actor.textId = 0x0B10;
        }
        player->actor.textId = this->actor.textId;
        return;
    }

    Actor_RequestToTalk(&this->actor, play);
    // Prime the vanilla "accept trade item" slot so Link can offer trade items at her.
    // EXCH_ITEM_BLUE_FIRE is upstream's choice; it is a harmless placeholder (nothing in
    // Lost Woods / Sacred Forest Meadow actually consumes blue fire from Link) and tells
    // the engine the NPC accepts trade exchange attempts at all. Cleared once Beat 1.3
    // marks the trade done.
    if (!TrifoolBeat1_IsBetaKokiriDone()) {
        player->exchangeItemId = EXCH_ITEM_BLUE_FIRE;
    }
}

static s32 TfBetaKokiri_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList,
                                         Vec3f* pos, Vec3s* rot, void* thisx) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;
    if (limbIndex == TF_BK_LIMB_HEAD) {
        TfBetaKokiri_SetBank(this, play, this->headBankIdx, 1);
        OPEN_DISPS(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeTextureIndex]));
        CLOSE_DISPS(play->state.gfxCtx);
        *dList = object_oE9_DL_000800;
        // Apply the Npc_TrackPoint head rot on top of the skeleton pose so she visibly
        // looks at Link. Upstream swapped Y onto X and X onto Z here; the beta skeleton's
        // head bone is set up the same way the vanilla Kokiri head is.
        rot->x += this->interactInfo.headRot.y;
        rot->z += this->interactInfo.headRot.x;
    }
    return false;
}

static void TfBetaKokiri_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot,
                                      void* thisx) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;
    if (limbIndex == TF_BK_LIMB_HEAD) {
        static Vec3f sHeadBoneToCenter = { 700.0f, 0.0f, 0.0f };
        TfBetaKokiri_SetBank(this, play, this->bodyBankIdx, 1);
        Matrix_MultVec3f(&sHeadBoneToCenter, &this->actor.focus.pos);
    }
}

void TfBetaKokiri_Update(Actor* thisx, PlayState* play) {
    TfBetaKokiri_UpdateWaitTalk(thisx, play);
}

void TfBetaKokiri_Draw(Actor* thisx, PlayState* play) {
    TfBetaKokiri* this = (TfBetaKokiri*)thisx;
    if (this->initState != 2) {
        return;
    }
    TfBetaKokiri_SetBank(this, play, this->bodyBankIdx, 1);
    OPEN_DISPS(play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 20, 110, 50, 255);
    CLOSE_DISPS(play->state.gfxCtx);
    SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                      TfBetaKokiri_OverrideLimbDraw, TfBetaKokiri_PostLimbDraw, this);
}
