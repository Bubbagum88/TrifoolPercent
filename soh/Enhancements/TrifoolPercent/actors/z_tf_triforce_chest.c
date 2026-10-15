/*
 * TriforceChest: the beta Triforce chest that opens at the start of the Triforce Room cutscene.
 *
 * Ported from Triforce% actor/TriforceChest/triforcechest.c (Triforce% team, used with credit under the
 * Triforce% license). Skeleton and animation are loaded from the TrifoolPercent archive.
 */

#include "../TrifoolPercent.h"

#define OPEN_DELAY 60
#define ANIM_LEN 50
#define TIMEWARP_SCALE 0.1f

void TfTriforceChest_Init(Actor* thisx, PlayState* play) {
    TfTriforceChest* this = (TfTriforceChest*)thisx;

    Actor_SetScale(&this->actor, 1.8f);
    SkelAnime_Init(play, &this->skelAnime, (SkeletonHeader*)gTrifoolTriforceChestSkel,
                   (AnimationHeader*)gTrifoolTriforceChestOpenAnim, NULL, NULL, 0);
    Animation_Change(&this->skelAnime, (AnimationHeader*)gTrifoolTriforceChestOpenAnim, 0.0f, 0.0f, 0.0f,
                     ANIMMODE_ONCE, 0.0f);
    this->state = 0;
    this->frame = 0;
    if (this->actor.params != 0) {
        // Already open
        this->state = 1;
        this->skelAnime.curFrame = this->skelAnime.endFrame = ANIM_LEN;
    }
}

void TfTriforceChest_Destroy(Actor* thisx, PlayState* play) {
    TfTriforceChest* this = (TfTriforceChest*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
}

void TfTriforceChest_Update(Actor* thisx, PlayState* play) {
    TfTriforceChest* this = (TfTriforceChest*)thisx;
    // The room's Demo_Effect time warp pillar is shrunk down to a beam coming out of the chest.
    Actor* timewarp = Actor_Find(&play->actorCtx, ACTOR_DEMO_EFFECT, ACTORCAT_BG);

    if (timewarp != NULL) {
        timewarp->scale.x = TIMEWARP_SCALE;
        timewarp->scale.z = TIMEWARP_SCALE;
    }
    if (this->state == 0 && this->frame >= OPEN_DELAY) {
        Animation_Change(&this->skelAnime, (AnimationHeader*)gTrifoolTriforceChestOpenAnim, 1.0f, 0.0f, ANIM_LEN,
                         ANIMMODE_ONCE, 0.0f);
        this->state = 1;
    }
    if (this->frame < 0xFF) {
        ++this->frame;
    }
    SkelAnime_Update(&this->skelAnime);
}

void TfTriforceChest_Draw(Actor* thisx, PlayState* play) {
    TfTriforceChest* this = (TfTriforceChest*)thisx;

    Gfx_SetupDL_25Opa(play->state.gfxCtx);
    SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, NULL, NULL, &this->actor);
}
