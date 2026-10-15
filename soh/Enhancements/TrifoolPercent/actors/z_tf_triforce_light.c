/*
 * TriforceLight: the beam of light that pours out of the Triforce chest as it opens.
 *
 * Ported from Triforce% actor/TriforceLight/triforcelight.c (Triforce% team, used with credit under the
 * Triforce% license). The original advanced its animation from the draw function; here it runs in Update so
 * the speed does not depend on SoH's frame interpolation.
 */

#include "../TrifoolPercent.h"

#define OPEN_DELAY 60
#define ANIM_LEN 50
#define DISAPPEAR_FRAME 380

void TfTriforceLight_Init(Actor* thisx, PlayState* play) {
    TfTriforceLight* this = (TfTriforceLight*)thisx;

    Actor_SetScale(&this->actor, 1.8f);
    SkelAnime_InitFlex(play, &this->skelAnime, (FlexSkeletonHeader*)gTrifoolTriforceLightSkel,
                       (AnimationHeader*)gTrifoolTriforceLightAnim, NULL, NULL, 0);
    this->state = 0;
    this->frame = 0;
}

void TfTriforceLight_Destroy(Actor* thisx, PlayState* play) {
    TfTriforceLight* this = (TfTriforceLight*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
}

void TfTriforceLight_Update(Actor* thisx, PlayState* play) {
    TfTriforceLight* this = (TfTriforceLight*)thisx;

    if (this->frame < OPEN_DELAY) {
        // Hold on the first frame until the chest opens
        Animation_Change(&this->skelAnime, (AnimationHeader*)gTrifoolTriforceLightAnim, 1.0f, 0.0f, 0.0f,
                         ANIMMODE_LOOP, 0.0f);
    } else if (this->frame == OPEN_DELAY) {
        this->state = 1;
    } else if (this->frame == OPEN_DELAY + ANIM_LEN - 1) {
        this->state = 2;
    } else if (this->frame == DISAPPEAR_FRAME) {
        this->state = 3;
    }
    if (this->state < 3) {
        ++this->frame;
    }
    if (this->state < 2) {
        SkelAnime_Update(&this->skelAnime);
    }
}

void TfTriforceLight_Draw(Actor* thisx, PlayState* play) {
    TfTriforceLight* this = (TfTriforceLight*)thisx;

    if (this->state >= 3) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);
    POLY_XLU_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                                       this->skelAnime.dListCount, NULL, NULL, &this->actor, POLY_XLU_DISP);
    CLOSE_DISPS(play->state.gfxCtx);
}
