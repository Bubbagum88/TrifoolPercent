/*
 * BetaGreatFairy: the fairy that rises from the Unicorn Fountain to teach the Overture of Sages.
 *
 * Ported from Triforce% actor/BetaGreatFairy/betagreatfairy.c (Triforce% team, used with credit under the
 * Triforce% license). The state machine, timing, particle spawns and audio cues match the original.
 *
 * Three things the original relied on are not in Triforce%'s published repo and are handled differently
 * here:
 *
 * 1. The compiled zobj (DL_BETAGREATFAIRY, DL_BETAGREATFAIRY_FLOWER) was never published, so the fairy's
 *    mesh is not drawn. The particles still spawn, so a column of golden sparkles rises from the fountain
 *    even without the body.
 * 2. UnicornFountain_scene_header00_cutscene is a reference to the scene's own cutscene, which has not
 *    been converted to SoH macros yet. The trigger that would play it is left as a stub and the state
 *    machine advances itself on a timer, so the proof of concept still demonstrates rise -> float ->
 *    fall end to end.
 * 3. Statics_GiveOvertureOfSages depends on Triforce%'s custom ocarina-song system. Not ported yet; the
 *    actor sits in state 2 (floating) after the rise instead of granting a song.
 */

#include "../TrifoolPercent.h"

#define RISE_FRAMES 40
#define SPIRAL_ROTATIONS 4
#define LEAN_FORWARD 0x1000
#define LEAN_SHIFT_Z 30.0f
#define LEAN_RATIO 0.15f
#define CYCLE_PERIOD 60.0f
#define CYCLE_HEIGHT 10.0f
#define FLOAT_CENTER_HEIGHT 60.0f

#define SCALE_TINY 0.001f
#define SCALE_MAIN 0.085f

// Debug auto-start: without the cutscene, state 0 would never advance. Rise after this many frames.
#define AUTO_RISE_FRAMES 60

static Vec3f sKira2Velocity = { 0.0f, 0.0f, 0.0f };
static Vec3f sKira2Accel = { 0.0f, 0.0f, 0.0f };
static Color_RGBA8 sKiraPrimColor = { 240, 255, 80, 255 };
static Color_RGBA8 sKiraEnvColor = { 200, 230, 0, 0 };
#define KIRA1_FRAMES 10

void TfBetaGreatFairy_Init(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;
    this->state = 0;
    this->frames = 0;
    this->ratio = 0.0f;
    this->lastFloatY = FLOAT_CENTER_HEIGHT;
    this->wingtip.x = this->wingtip.y = this->wingtip.z = 0.0f;
    this->lastWingtip = this->wingtip;
}

void TfBetaGreatFairy_Destroy(Actor* thisx, PlayState* play) {
}

void TfBetaGreatFairy_Update(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;

    this->actor.world.pos = this->actor.home.pos;
    this->actor.shape.rot.x = 0;
    this->actor.shape.rot.y = 0;
    this->actor.shape.rot.z = 0;

    if (this->state == 0) {
        // Hidden, waiting to rise. The original waited for NPC action from the room's cutscene; we auto-advance.
        Actor_SetScale(&this->actor, SCALE_TINY);
        this->actor.world.pos.y -= 1000.0f;
        this->lastFloatY = FLOAT_CENTER_HEIGHT;
        ++this->frames;
        if (this->frames >= AUTO_RISE_FRAMES) {
            this->frames = 0;
            ++this->state;
        }
    } else if (this->state == 2) {
        // Floating. Original handed over to the cutscene for the song give and then fall; we stay floating.
        Actor_SetScale(&this->actor, SCALE_MAIN);
        float y = CYCLE_HEIGHT * sinf((float)this->frames * (2.0f * 3.14159f / CYCLE_PERIOD));
        this->actor.world.pos.z -= LEAN_SHIFT_Z;
        this->actor.world.pos.y += y + FLOAT_CENTER_HEIGHT;
        this->lastFloatY = this->actor.world.pos.y;
        this->actor.shape.rot.x = LEAN_FORWARD;
        ++this->frames;
        if (this->frames >= CYCLE_PERIOD) this->frames = 0;
    } else if (this->state == 1 || this->state == 3) {
        // Rising or falling.
        if (this->frames == 0) {
            Audio_PlayActorSound2(&this->actor, NA_SE_VO_FR_LAUGH_0);
        } else if (this->frames == 1) {
            Audio_PlayActorSound2(&this->actor,
                                  this->state == 3 ? NA_SE_EV_GREAT_FAIRY_VANISH : NA_SE_EV_GREAT_FAIRY_APPEAR);
        }
        this->ratio = (float)this->frames / (float)RISE_FRAMES;
        if (this->state == 1) this->ratio = 1.0f - this->ratio;

        float scale = this->ratio * (SCALE_TINY - SCALE_MAIN) + SCALE_MAIN;
        Actor_SetScale(&this->actor, scale);
        this->actor.world.pos.y += (1.0f - this->ratio) * this->lastFloatY;
        float rot = (this->ratio * this->ratio) * SPIRAL_ROTATIONS * 0x10000;
        this->actor.shape.rot.y = (s16)((s32)rot & 0xFFFF);
        if (this->state == 1) this->actor.shape.rot.y = -this->actor.shape.rot.y;

        if (this->ratio < LEAN_RATIO) {
            float leanratio = 1.0f - (this->ratio / LEAN_RATIO);
            this->actor.shape.rot.x = (s16)((float)LEAN_FORWARD * leanratio);
            this->actor.world.pos.z -= leanratio * LEAN_SHIFT_Z;
        }

        this->lastWingtip = this->wingtip;
        float rad = (130.0f / SCALE_MAIN) * scale;
        float height = (150.0f / SCALE_MAIN) * scale;
        this->wingtip.x = rad * Math_CosS(this->actor.shape.rot.y);
        this->wingtip.z = rad * -Math_SinS(this->actor.shape.rot.y);
        this->wingtip.y = height;

        ++this->frames;
        if (this->frames == RISE_FRAMES) {
            this->frames = 0;
            this->state = (this->state + 1) & 3;
        }
    }
}

void TfBetaGreatFairy_Draw(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;

    if (this->state == 0) return;

    // The fairy's mesh (DL_BETAGREATFAIRY, DL_BETAGREATFAIRY_FLOWER) is not available — only particles are drawn.

    if (this->state == 2) {
        const s32 nspawn = 1;
        for (s32 i = 0; i < nspawn; ++i) {
            Vec3f pos = this->actor.world.pos;
            pos.x += 160.0f * (Rand_ZeroOne() - 0.5f);
            pos.y += 120.0f * Rand_ZeroOne();
            pos.z += 70.0f * Rand_ZeroOne() + 10.0f;
            EffectSsKiraKira_SpawnFocused(play, &pos, &sKira2Velocity, &sKira2Accel, &sKiraPrimColor, &sKiraEnvColor,
                                          3000, 10);
        }
    } else {
        s32 nspawn_perside = 2;
        float size = (1.0f - this->ratio) * 20.0f;
        Vec3f vel, accel;
        vel.x = (this->wingtip.x - this->lastWingtip.x) * 0.1f;
        vel.y = (this->wingtip.y - this->lastWingtip.y) * 0.1f;
        vel.z = (this->wingtip.z - this->lastWingtip.z) * 0.1f;
        accel.x = (1.0f / KIRA1_FRAMES) * -vel.x;
        accel.y = (1.0f / KIRA1_FRAMES) * -vel.y - 0.4f;
        accel.z = (1.0f / KIRA1_FRAMES) * -vel.z;
        for (s32 i = 0; i < nspawn_perside; ++i) {
            Vec3f mainpos = this->actor.world.pos;
            mainpos.x += size * (Rand_ZeroOne() - 0.5f);
            mainpos.y += size * (Rand_ZeroOne() - 0.5f);
            mainpos.z += size * (Rand_ZeroOne() - 0.5f);
            Vec3f pos = mainpos;
            pos.x += this->wingtip.x;
            pos.y += this->wingtip.y;
            pos.z += this->wingtip.z;
            EffectSsKiraKira_SpawnFocused(play, &pos, &vel, &accel, &sKiraPrimColor, &sKiraEnvColor,
                                          (1.0f - this->ratio) * 3000.0f, KIRA1_FRAMES);
            pos.x = mainpos.x - this->wingtip.x;
            pos.z = mainpos.z - this->wingtip.z;
            vel.x = -vel.x;
            vel.z = -vel.z;
            accel.x = -accel.x;
            accel.z = -accel.z;
            EffectSsKiraKira_SpawnFocused(play, &pos, &vel, &accel, &sKiraPrimColor, &sKiraEnvColor,
                                          (1.0f - this->ratio) * 3000.0f, KIRA1_FRAMES);
            vel.x = -vel.x;
            vel.z = -vel.z;
            accel.x = -accel.x;
            accel.z = -accel.z;
        }
    }
}
