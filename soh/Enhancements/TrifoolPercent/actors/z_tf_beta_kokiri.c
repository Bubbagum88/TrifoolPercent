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

// SoH extracts the vanilla beta Kokiri object slots as __OTR__ resource paths; the
// SkelAnime / Animation / gfx runtime detects the string marker and loads via
// ResourceMgr, same way all other SoH actors reach their object data.
//
// Note on eye textures: zeldaret/oot's object_oE9.xml descriptor marks three CI8 32x32
// frames (0x0E90 open, 0x1290 half, 0x1690 closed) spanning offsets 0x0E90..0x1A90. SoH's
// own YML descriptor splits that range differently -- Tex_000E90 is published as a proper
// CI8 texture, but 0x1290..0x1A90 is published as a single Blob_001290 (0x800 bytes), and
// the 0x1690 frame does not get a standalone symbol. Rather than reach into the blob at a
// runtime offset (which the ResourceMgr-managed pointer we hold does not support without
// loading the raw buffer ourselves), Beat 1.1 falls back to the open-eye texture for all
// three blink states. She stops blinking; the actor still spawns, animates and talks.
// Follow-up: ship a mod OTR override that re-extracts object_oE9 with Tex_001290 and
// Tex_001690 as separate CI8 textures, then restore the three-frame cycle below.
#define dobject_oE4_Skel_000260 "__OTR__objects/object_oE4/object_oE4_Skel_000260"
#define dobject_oE9_DL_000800 "__OTR__objects/object_oE9/object_oE9_DL_000800"
#define dobject_oE9_Tex_000E90 "__OTR__objects/object_oE9/object_oE9_Tex_000E90"
#define dobject_oE_anime_Anim_00001034 "__OTR__objects/object_oE_anime/object_oE_anime_Anim_00001034"
static const ALIGN_ASSET(2) char object_oE4_Skel_000260[] = dobject_oE4_Skel_000260;
static const ALIGN_ASSET(2) char object_oE9_DL_000800[] = dobject_oE9_DL_000800;
static const ALIGN_ASSET(2) char object_oE9_Tex_000E90[] = dobject_oE9_Tex_000E90;
static const ALIGN_ASSET(2) char object_oE_anime_Anim_00001034[] = dobject_oE_anime_Anim_00001034;

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

static ColliderCylinderInitType1 sCylinderInit = {
    { COLTYPE_HIT0, AT_NONE, AC_NONE, OC1_ON | OC1_TYPE_PLAYER, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0x00000000, 0x00, 0x00 },
      TOUCH_NONE, BUMP_NONE, OCELEM_ON, },
    { 20, 80, 0, { 0, 0, 0 } },
};

// All three blink slots point at the open-eye texture until the OTR extraction splits
// object_oE9_Blob_001290 back into Tex_001290 (half) and Tex_001690 (closed). See the
// note above the resource-name declarations.
static const void* sEyeTextures[3] = {
    (const void*)object_oE9_Tex_000E90,
    (const void*)object_oE9_Tex_000E90,
    (const void*)object_oE9_Tex_000E90,
};

static void TfBetaKokiri_SetBank(TfBetaKokiri* this, PlayState* play, s32 bank, s32 alsoRCP) {
    void* segment = play->objectCtx.status[bank].segment;
    gSegments[6] = VIRTUAL_TO_PHYSICAL(segment);
    if (alsoRCP) {
        OPEN_DISPS(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)segment);
        CLOSE_DISPS(play->state.gfxCtx);
    }
}

static void TfBetaKokiri_UpdateEyes(TfBetaKokiri* this) {
    if (this->blinkTimer == 0) {
        this->blinkTimer = (uint8_t)Rand_S16Offset(60, 60);
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

    // params == 1: onboarding variant used by Beat 1.0 to spawn the kick-off Kokiri by
    // Link's house. She is always alive while her CVar is on and uses her own text id;
    // she does not gate on the Lost Woods path or carry the trade branch.
    bool onboarding = (this->actor.params == 1);
    if (!onboarding && !TrifoolBeat1_IsBetaKokiriSpawned()) {
        Actor_Kill(&this->actor);
        return;
    }

    this->initState = 0;
    this->blinkTimer = 0;
    this->eyeTextureIndex = 0;
    this->actor.textId = onboarding ? 0x0100 : 0x0B10;

    // NPC lock-on, not enemy. ACTOR_FLAG_FRIENDLY makes Z-target treat her as a talkable
    // NPC (green reticle, no attack prompt); ACTOR_FLAG_TARGETABLE was already set at
    // ActorDB registration time. targetMode 6 is the vanilla "friendly NPC" range tier
    // (what shopkeepers and Kokiri children use).
    this->actor.flags |= ACTOR_FLAG_FRIENDLY;
    this->actor.targetMode = 6;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, TF_BK_SHADOW_SIZE);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);

    // Sacred Forest Meadow's vanilla object list does not contain the three beta Kokiri
    // slots (0x34 / 0x4C / 0x52), so a plain Object_GetIndex lookup returns -1 for all
    // three and the actor dies on frame one. Upstream Triforce% patched the scene file to
    // include them; our ActorDB-based port instead requests each slot at runtime via
    // Object_Spawn, which allocates it in the scene's object context and returns the bank
    // index. TfBetaKokiri_ObjectsLoaded still gates the SkelAnime bring-up on
    // Object_IsLoaded, same as before.
    this->animBankIdx = Object_Spawn(&play->objectCtx, TF_BK_OBJ_ANIM);
    this->bodyBankIdx = Object_Spawn(&play->objectCtx, TF_BK_OBJ_BODY);
    this->headBankIdx = Object_Spawn(&play->objectCtx, TF_BK_OBJ_HEAD);
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
    Npc_TrackPoint(&this->actor, &this->interactInfo, TF_BK_HEAD_FOLLOW_PRESET,
                   TF_BK_HEAD_FOLLOW_MODE);
}

static s32 TfBetaKokiri_UpdateCommon(TfBetaKokiri* this, PlayState* play) {
    if (!TfBetaKokiri_ObjectsLoaded(this, play)) {
        return -1;
    }
    if (this->initState == 1) {
        this->initState = 2;
        TfBetaKokiri_SetBank(this, play, this->bodyBankIdx, 0);
        SkelAnime_Init(play, &this->skelAnime, (SkeletonHeader*)object_oE4_Skel_000260, NULL,
                       this->jointTable, this->morphTable, TF_BK_NUM_LIMBS);
        TfBetaKokiri_SetBank(this, play, this->animBankIdx, 0);
        Animation_Change(&this->skelAnime, (AnimationHeader*)object_oE_anime_Anim_00001034, 1.0f, 0.0f,
                         Animation_GetLastFrame((AnimationHeader*)object_oE_anime_Anim_00001034),
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

    bool onboarding = (this->actor.params == 1);

    if (Actor_ProcessTalkRequest(&this->actor, play)) {
        if (onboarding) {
            // The onboarding Kokiri always speaks text 0x0100. The 3-choice box lives in
            // the CustomMessage loader; the picked branch is applied in Beat1_0Onboarding
            // on close. Nothing to switch on here.
            this->actor.textId = 0x0100;
            player->actor.textId = this->actor.textId;
            return;
        }
        s8 exch = player->exchangeItemId;
        if (TrifoolBeat1_IsBetaKokiriDone()) {
            // Beat 1.3 will arm this once the Butterfly trade is accepted; until then it
            // never fires, matching upstream's pre-trade behaviour.
            this->actor.textId = 0x0B16;
        } else if (exch == EXCH_ITEM_BUG) {
            // Vanilla "Bug in a Bottle" (child trade slot). She rejects it as a crawly.
            this->actor.textId = 0x0B12;
            Sfx_PlaySfxCentered(NA_SE_SY_ERROR);
        } else if (exch == TRIFOOL_EXCH_ITEM_BUTTERFLY) {
            // Beat 1.3: the Butterfly rides unused slot 0x68 and its own unused exchange
            // id 0x18, so SoH never confuses it with a vanilla trade item. When Link
            // offers the Butterfly bottle, Beta Kokiri accepts it: swap the bottle for
            // an empty one and the Odd Potion give lands next tick.
            this->actor.textId = 0x0B13;
            Sfx_PlaySfxCentered(NA_SE_SY_TRE_BOX_APPEAR);
            Player_UpdateBottleHeld(play, player, ITEM_BOTTLE, PLAYER_IA_BOTTLE);
            TrifoolBeat1_SetBetaKokiriDone();
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

    Actor_OfferTalkNearColChkInfoCylinder(&this->actor, play);
    // Prime the vanilla "accept trade item" slot so Link can offer trade items at her.
    // EXCH_ITEM_BLUE_FIRE is upstream's choice; it is a harmless placeholder (nothing in
    // Lost Woods / Sacred Forest Meadow actually consumes blue fire from Link) and tells
    // the engine the NPC accepts trade exchange attempts at all. Cleared once Beat 1.3
    // marks the trade done. The onboarding variant doesn't need a trade channel.
    if (!onboarding && !TrifoolBeat1_IsBetaKokiriDone()) {
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
                   (uintptr_t)SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeTextureIndex]));
        CLOSE_DISPS(play->state.gfxCtx);
        *dList = (Gfx*)object_oE9_DL_000800;
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
