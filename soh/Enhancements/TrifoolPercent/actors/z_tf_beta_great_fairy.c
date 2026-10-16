/*
 * BetaGreatFairy: the fairy that rises from the Unicorn Fountain to teach the Overture of Sages.
 *
 * Ported from Triforce% actor/BetaGreatFairy/betagreatfairy.c (Triforce% team, used with credit under the
 * Triforce% license). State machine, timing, particle spawns and audio cues match the original.
 *
 * Three things the original relied on are not in Triforce%'s published repo and are handled differently
 * here:
 *
 * 1. The compiled zobj (DL_BETAGREATFAIRY, DL_BETAGREATFAIRY_FLOWER) was never published, so the fairy's
 *    mesh is not drawn. The particles still spawn, so a column of golden sparkles rises from the fountain
 *    even without the body.
 * 2. UnicornFountain_scene_header00_cutscene (the scene's own cutscene) has not been converted to SoH
 *    macros yet. Instead of CHECK_NPC_ACTION(FAIRY_SLOT, N), this file drives the state machine on its
 *    own: a short delay in state 0 before the rise, textbox 0x0D23 (Overture backstory) then 0x0074
 *    (You learned Overture of Sages) during the float, and auto-advance to the fall when both close.
 *    This matches upstream's observable sequence (rise -> text -> song -> fall) without the camera
 *    choreography; the camera port is a follow-up slice.
 * 3. Statics_GiveOvertureOfSages depends on Triforce%'s custom ocarina-song system. Not ported yet;
 *    the textbox stands in for the song gift, and no song is actually learned.
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

// Dialogue text ids, lifted from Triforce% text/messages.c.
#define TEXTID_OVERTURE_BACKSTORY 0x0D23 // "Play this song before the blade of destiny..."
#define TEXTID_OVERTURE_LEARNED   0x0074 // "You have learned the Overture of Sages!!"

// How many frames the fairy floats before the backstory textbox opens, and how long the "learned"
// title-card-style textbox hangs after the backstory closes.
#define FLOAT_FRAMES_BEFORE_TEXT 90
#define LEARNED_HOLD_FRAMES      150

// Terminal state the glue (UnicornFountain.cpp) polls to fade back to Kokiri Forest.
#define STATE_DONE 4

// Dialogue sub-state (uses this->blinkTimer as a scratch counter for the dialogue phase).
#define DIALOGUE_IDLE       0 // still floating, haven't opened the first textbox
#define DIALOGUE_BACKSTORY  1 // backstory textbox showing
#define DIALOGUE_LEARNED    2 // "learned" textbox showing
#define DIALOGUE_DONE       3 // both closed, ready to fall

static Vec3f sKira2Velocity = { 0.0f, 0.0f, 0.0f };
static Vec3f sKira2Accel = { 0.0f, 0.0f, 0.0f };
static Color_RGBA8 sKiraPrimColor = { 240, 255, 80, 255 };
static Color_RGBA8 sKiraEnvColor = { 200, 230, 0, 0 };
#define KIRA1_FRAMES 10

static bool sFairyDone = false;

bool TrifoolUnicornFountain_IsFairyDone(void) {
    return sFairyDone;
}

void TrifoolUnicornFountain_ClearFairyDone(void) {
    sFairyDone = false;
}

void TfBetaGreatFairy_Init(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;
    this->state = 0;
    this->frames = 0;
    this->dialoguePhase = DIALOGUE_IDLE;
    this->floatFrames = 0;
    this->ratio = 0.0f;
    this->lastFloatY = FLOAT_CENTER_HEIGHT;
    this->wingtip.x = this->wingtip.y = this->wingtip.z = 0.0f;
    this->lastWingtip = this->wingtip;
    sFairyDone = false;
}

void TfBetaGreatFairy_Destroy(Actor* thisx, PlayState* play) {
}

// A textbox counts as finished for the fairy's purposes when msgMode has returned to NONE *after*
// the textbox actually opened; floatFrames gives the per-phase ramp so we don't mistake the frame
// right after Message_StartTextbox (still NONE for one tick) for the dismiss.
static s32 TextboxClosedAfter(PlayState* play, u8 framesInPhase, u8 minFrames) {
    return framesInPhase >= minFrames && play->msgCtx.msgMode == MSGMODE_NONE;
}

#define TEXTBOX_MIN_OPEN_FRAMES 10

void TfBetaGreatFairy_Update(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;

    this->actor.world.pos = this->actor.home.pos;
    this->actor.shape.rot.x = 0;
    this->actor.shape.rot.y = 0;
    this->actor.shape.rot.z = 0;

    if (this->state == STATE_DONE) {
        // Hidden terminal: UnicornFountain.cpp has fired its scene transition out and is waiting on it.
        Actor_SetScale(&this->actor, SCALE_TINY);
        this->actor.world.pos.y -= 1000.0f;
        return;
    }

    if (this->state == 0) {
        // Hidden, waiting to rise. The original waited for NPC action from the room's cutscene; we auto-advance.
        // Hold the counter while a cutscene is running so the fountain's entry camera (UnicornFountainCs.c) can
        // land before the fairy rises - otherwise the pan and the rise race.
        Actor_SetScale(&this->actor, SCALE_TINY);
        this->actor.world.pos.y -= 1000.0f;
        this->lastFloatY = FLOAT_CENTER_HEIGHT;
        if (play->csCtx.state == CS_STATE_IDLE) {
            ++this->frames;
            if (this->frames >= AUTO_RISE_FRAMES) {
                this->frames = 0;
                ++this->state;
            }
        }
    } else if (this->state == 2) {
        // Floating. Upstream handed over to the cutscene for the song give and then fall. Without that cutscene we
        // drive the dialogue inline: float a bit, open the "Overture" backstory textbox, follow with the "learned"
        // textbox, then advance to the fall. State 3 wraps to STATE_DONE below, which the glue uses to fade back
        // out to Kokiri Forest.
        Actor_SetScale(&this->actor, SCALE_MAIN);
        float y = CYCLE_HEIGHT * sinf((float)this->frames * (2.0f * 3.14159f / CYCLE_PERIOD));
        this->actor.world.pos.z -= LEAN_SHIFT_Z;
        this->actor.world.pos.y += y + FLOAT_CENTER_HEIGHT;
        this->lastFloatY = this->actor.world.pos.y;
        this->actor.shape.rot.x = LEAN_FORWARD;
        ++this->frames;
        if (this->frames >= CYCLE_PERIOD) this->frames = 0;

        if (this->floatFrames < 0xFF) ++this->floatFrames;
        switch (this->dialoguePhase) {
            case DIALOGUE_IDLE:
                if (this->floatFrames >= FLOAT_FRAMES_BEFORE_TEXT && play->msgCtx.msgMode == MSGMODE_NONE) {
                    Message_StartTextbox(play, TEXTID_OVERTURE_BACKSTORY, &this->actor);
                    this->dialoguePhase = DIALOGUE_BACKSTORY;
                    this->floatFrames = 0;
                }
                break;
            case DIALOGUE_BACKSTORY:
                if (TextboxClosedAfter(play, this->floatFrames, TEXTBOX_MIN_OPEN_FRAMES)) {
                    Message_StartTextbox(play, TEXTID_OVERTURE_LEARNED, &this->actor);
                    this->dialoguePhase = DIALOGUE_LEARNED;
                    this->floatFrames = 0;
                }
                break;
            case DIALOGUE_LEARNED:
                if (TextboxClosedAfter(play, this->floatFrames, LEARNED_HOLD_FRAMES)) {
                    this->dialoguePhase = DIALOGUE_DONE;
                    this->frames = 0;
                    this->state = 3; // start falling
                }
                break;
            default:
                break;
        }
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
            if (this->state == 1) {
                // Rise finished -> float.
                this->state = 2;
            } else {
                // Fall finished -> terminal state; the glue warps Link out.
                this->state = STATE_DONE;
                sFairyDone = true;
            }
        }
    }
}

void TfBetaGreatFairy_Draw(Actor* thisx, PlayState* play) {
    TfBetaGreatFairy* this = (TfBetaGreatFairy*)thisx;

    if (this->state == 0 || this->state == STATE_DONE) return;

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
