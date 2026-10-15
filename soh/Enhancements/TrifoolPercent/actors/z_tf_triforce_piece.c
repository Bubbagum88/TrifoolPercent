/*
 * TriforcePiece: the three pieces that rise, combine, sink and vanish into Link in the Triforce Room.
 *
 * Ported from Triforce% actor/TriforcePiece/triforcepiece.c (Sauraen and the Triforce% team, used with credit
 * under the Triforce% license). The state machine and tables are unchanged; only the engine calls are renamed
 * for the SoH decomp and the display list is loaded from the TrifoolPercent archive.
 */

#include "../TrifoolPercent.h"

// The Triforce Room cutscene drives NPC action list 40, which the engine stores in npcActions[1].
#define NPC_ACTION_SLOT 1

#define TFCOLOR_R 255
#define TFCOLOR_G 217
#define TFCOLOR_B 100

#define STATE_START 0
#define STATE_RISE 1
#define STATE_WAIT0 2
#define STATE_COMBINE 3
#define STATE_WAIT1 4
#define STATE_SINK 5
#define STATE_WAIT2 6
#define STATE_DISAP 7
#define STATE_LINK 8

#define STATE_MODE_LAST 0
#define STATE_MODE_IMM 1
#define STATE_MODE_SMOOTH 2

// clang-format off
static const u8 sStatesMode[] = {
       1,   2,   2,   2,   2,  2,   2,  1,  0
};
// -- ST  RIS   W0  CMB   W1  SNK  W2  DS  LK
static const u8 sStatesFrames[] = {
      57,  88,  95,  60,  10,  60, 10, 57,  0
};
// These have an extra value at the beginning so we can interpolate from last
// to current without worrying about crashes
static const u8 sStatesHighPieceY[] = {
    0, 0, 125, 125, 117, 117,  82, 82,  0, 76
};
static const u8 sStatesLowPiecesY[] = {
    0, 0,  85,  85,  95,  95,  65, 65,  0, 65
};
// -- ST  RIS   W0  CMB   W1  SNK  W2  DS  LK
static const u8 sStatesLowPiecesX[] = {
    0, 0,  25,  25,  12,  12,  10, 10, 10,  6
};
static const u8 sStatesZ[] = {
    0, 0,   0,   0,  13,  13,  13, 13,  0, 50
};
// -- ST  RIS   W0  CMB   W1  SNK  W2  DS  LK
static const u8 sStatesScale[] = {
   64,64, 128, 128, 128, 128, 100,100,  1, 64
};
static const s8 sStatesBrightness[] = {
  -30,-30, 60,  60,  60,  60,  60, 60,  0, 60
};

static const u8 sSmoothTable[] = {
    0x00, 0x0A, 0x1C, 0x48, 0x80, 0xB8, 0xE4, 0xF6, 0xFF
};
static const s16 sPiecesRot[] = {
    1200, -870, 0,
    -580, 0, 1107,
    0, 1337, 420
};
// clang-format on

static void TfTriforcePiece_SetPos(TfTriforcePiece* this, f32 x, f32 y, f32 z) {
    this->actor.world.pos.x = x;
    this->actor.world.pos.y = y;
    this->actor.world.pos.z = z;
    if (this->actor.params == TRIFOOL_TRIFORCE_POWER) {
        this->light.params.point.x = (s16)x;
        this->light.params.point.y = (s16)y - 12;
        this->light.params.point.z = (s16)z + 4;
    }
}

static void TfTriforcePiece_RotCombine(s16* r, s16 tbl, s32 framesLeft) {
    s32 rRemain = (s32)(*r);
    s32 d;

    if (tbl > 0 && rRemain > 0) {
        rRemain = -0x10000 + rRemain;
    } else if (tbl < 0 && rRemain < 0) {
        rRemain = 0x10000 + rRemain;
    }
    d = rRemain / framesLeft;
    if (framesLeft > 3) {
        // Do a little more than needed; will automatically slow down the last few frames.
        d <<= 1;
    }
    *r -= d;
}

void TfTriforcePiece_Init(Actor* thisx, PlayState* play) {
    TfTriforcePiece* this = (TfTriforcePiece*)thisx;

    this->theEndMode = this->actor.params >> 2;
    this->actor.params &= 3;
    if (this->actor.params == TRIFOOL_TRIFORCE_POWER) {
        Lights_PointNoGlowSetInfo(&this->light, 0, 0, 0, TFCOLOR_R, TFCOLOR_G, TFCOLOR_B, 2000);
        this->lightNode = LightContext_InsertLight(play, &play->lightCtx, &this->light);
    }
    this->state = this->theEndMode ? STATE_WAIT1 : STATE_START;
    this->frame = 0;
    TfTriforcePiece_SetPos(this, 0.0f, 0.0f, 0.0f);
}

void TfTriforcePiece_Destroy(Actor* thisx, PlayState* play) {
    TfTriforcePiece* this = (TfTriforcePiece*)thisx;

    if (this->actor.params == TRIFOOL_TRIFORCE_POWER) {
        LightContext_RemoveLight(play, &play->lightCtx, this->lightNode);
    }
}

void TfTriforcePiece_Update(Actor* thisx, PlayState* play) {
    TfTriforcePiece* this = (TfTriforcePiece*)thisx;
    f32 x, y, z, s, brightness, lastX, lastY, lastZ, lastS, lastBrightness;
    f32 m;
    s16 variable;
    u8 state, frame, frames, mode;
    const s16* rotTable;

    if ((play->csCtx.state != CS_STATE_IDLE) && (play->csCtx.npcActions[NPC_ACTION_SLOT] != NULL) &&
        (play->csCtx.npcActions[NPC_ACTION_SLOT]->action == 1)) {
        Actor_Kill(&this->actor);
        return;
    }

    variable = this->actor.params;
    if (variable >= 3) {
        variable = TRIFOOL_TRIFORCE_POWER;
    }
    state = this->state;
    frame = this->frame;
    frames = sStatesFrames[state];
    if (frames == 0) {
        frames = 1;
    }
    mode = sStatesMode[state];

    // Position, scale, brightness
    lastX = (f32)sStatesLowPiecesX[state];
    lastY = (f32)sStatesLowPiecesY[state];
    lastZ = (f32)sStatesZ[state];
    lastS = (f32)sStatesScale[state];
    lastBrightness = (f32)sStatesBrightness[state];
    x = (f32)sStatesLowPiecesX[state + 1]; // End of current state
    y = (f32)sStatesLowPiecesY[state + 1];
    z = (f32)sStatesZ[state + 1];
    s = (f32)sStatesScale[state + 1];
    brightness = (f32)sStatesBrightness[state + 1];
    if (this->theEndMode) {
        z = lastZ = 0.0f;
    }
    if (variable == TRIFOOL_TRIFORCE_COURAGE) {
        lastX = -lastX;
        x = -x;
    } else if (variable == TRIFOOL_TRIFORCE_POWER) {
        lastX = 0.0f;
        x = 0.0f;
        lastY = (f32)sStatesHighPieceY[state];
        y = (f32)sStatesHighPieceY[state + 1];
    }

    m = 1.0f;
    if (mode >= STATE_MODE_SMOOTH) {
        // Interpolate parameters
        u32 smoothIdx;
        f32 smoothStart, smoothEnd;

        m = (f32)((u32)frame << 3) / (f32)frames;
        smoothIdx = (u32)m;
        m -= (f32)smoothIdx; // fractional part
        smoothStart = (f32)sSmoothTable[smoothIdx];
        smoothEnd = (f32)sSmoothTable[smoothIdx + 1];
        m = (smoothEnd - smoothStart) * m + smoothStart;
        m *= 1.0f / 256.0f;
    }
    x = (x - lastX) * m + lastX;
    y = (y - lastY) * m + lastY;
    z = (z - lastZ) * m + lastZ;
    s = (s - lastS) * m + lastS;
    brightness = (brightness - lastBrightness) * m + lastBrightness;
    if (y < 40.0f || brightness < 0.0f) {
        brightness = 0.0f;
    }
    TfTriforcePiece_SetPos(this, x, y, z);
    Actor_SetScale(&this->actor, s * 2.5f * (1.0f / 128.0f));
    if (variable == TRIFOOL_TRIFORCE_POWER) {
        this->light.params.point.radius = (s16)brightness << 4;
    }

    // Rotation
    rotTable = &sPiecesRot[3 * variable];
    if (state == STATE_RISE || state == STATE_WAIT0) {
        // Free rotation
        this->actor.shape.rot.x += rotTable[0];
        this->actor.shape.rot.y += rotTable[1];
        this->actor.shape.rot.z += rotTable[2];
    } else if (state == STATE_COMBINE) {
        // Rotate back to upright
        s32 framesLeft = frames - frame;

        TfTriforcePiece_RotCombine(&this->actor.shape.rot.x, rotTable[0], framesLeft);
        TfTriforcePiece_RotCombine(&this->actor.shape.rot.y, rotTable[1], framesLeft);
        TfTriforcePiece_RotCombine(&this->actor.shape.rot.z, rotTable[2], framesLeft);
    } else {
        this->actor.shape.rot.x = 0;
        this->actor.shape.rot.y = 0;
        this->actor.shape.rot.z = 0;
    }

    // Advance the state machine
    if (!this->theEndMode && mode != STATE_MODE_LAST) {
        ++frame;
        if (frame >= frames) {
            frame = 0;
            this->state = state + 1;
        }
        this->frame = frame;
    }

    // The Ending scene shows the finished Triforce turned 40 degrees
    if (this->theEndMode) {
        const f32 sin40 = 0.6427876f;
        const f32 cos40 = 0.7660444f;
        f32 px = this->actor.world.pos.x;
        f32 pz = this->actor.world.pos.z;

        this->actor.shape.rot.y = (s16)((f32)0x4000 * (40.0f / 90.0f));
        this->actor.world.pos.x = cos40 * px + sin40 * pz;
        this->actor.world.pos.z = -sin40 * px + cos40 * pz;
    }
}

void TfTriforcePiece_Draw(Actor* thisx, PlayState* play) {
    TfTriforcePiece* this = (TfTriforcePiece*)thisx;

    if (this->state == STATE_START || this->state == STATE_DISAP) {
        return;
    }
    Gfx_DrawDListOpa(play, (Gfx*)gTrifoolTriforcePieceDL);
}
