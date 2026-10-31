/*
 * TfUnusedFlame: a billboard flame drawn from the unused OBJECT_FIRE slot (0x0F,
 * "Dancing Flame" per TCRF). Vanilla OoT ships the object (gFire0Tex..gFire7Tex, 8
 * ia8 32x64 frames) but no actor references it; this is the smallest actor that puts
 * the asset on screen. Used in Kokiri Forest as the "test entrance" marker outside
 * Link's house instead of a stock En_Light big blue flame.
 *
 * Design:
 *   - Request OBJECT_FIRE (0x0F) at Init via Object_GetIndex; wait for Object_IsLoaded
 *     before drawing (identical pattern to Beat 1.1 Beta Kokiri).
 *   - Add a flickering point light so the warp spot is still legible if the asset
 *     fails to render for any reason (e.g. the OTR extract left object_fire out).
 *   - Draw a camera-facing quad each frame, cycling gFire0Tex..gFire7Tex every 4
 *     game frames (~7.5 Hz) for the eight-frame flame loop upstream intended. The
 *     DL is built inline instead of invoking gFireDL -- we have the texture symbols
 *     from zeldaret/oot's object_fire.xml, but the DL body is not referenced from
 *     anywhere in retail code, so we trust the textures rather than guess what the
 *     DL expects at its boundary.
 *
 * Params: unused for now; the actor is spawned from TrifoolPercent.cpp glue only.
 */

#include "../TrifoolPercent.h"

#define TF_UNUSED_FLAME_OBJECT 0x0F // OBJECT_FIRE in vanilla OoT's object table

// SoH extracts object_fire's eight IA8 32x64 flame frames as __OTR__ resource paths
// (gFire0Tex..gFire7Tex). We pass the resource-name strings straight to the gfx macros
// below; the renderer resolves them via ResourceMgr, same as any other SoH texture.
#define dgFire0Tex "__OTR__objects/object_fire/gFire0Tex"
#define dgFire1Tex "__OTR__objects/object_fire/gFire1Tex"
#define dgFire2Tex "__OTR__objects/object_fire/gFire2Tex"
#define dgFire3Tex "__OTR__objects/object_fire/gFire3Tex"
#define dgFire4Tex "__OTR__objects/object_fire/gFire4Tex"
#define dgFire5Tex "__OTR__objects/object_fire/gFire5Tex"
#define dgFire6Tex "__OTR__objects/object_fire/gFire6Tex"
#define dgFire7Tex "__OTR__objects/object_fire/gFire7Tex"
static const ALIGN_ASSET(2) char gFire0Tex[] = dgFire0Tex;
static const ALIGN_ASSET(2) char gFire1Tex[] = dgFire1Tex;
static const ALIGN_ASSET(2) char gFire2Tex[] = dgFire2Tex;
static const ALIGN_ASSET(2) char gFire3Tex[] = dgFire3Tex;
static const ALIGN_ASSET(2) char gFire4Tex[] = dgFire4Tex;
static const ALIGN_ASSET(2) char gFire5Tex[] = dgFire5Tex;
static const ALIGN_ASSET(2) char gFire6Tex[] = dgFire6Tex;
static const ALIGN_ASSET(2) char gFire7Tex[] = dgFire7Tex;

static const void* sFireFrames[8] = {
    (const void*)gFire0Tex, (const void*)gFire1Tex,
    (const void*)gFire2Tex, (const void*)gFire3Tex,
    (const void*)gFire4Tex, (const void*)gFire5Tex,
    (const void*)gFire6Tex, (const void*)gFire7Tex,
};

// A billboard quad sized to roughly match an En_Light "large blue flame" (params 0x0003).
// Texture is 32x64; the quad is 48x96 world units so it reads at the same scale as the
// vanilla flame it replaces.
static const Vtx sFlameVtx[4] = {
    { { { -24,  0, 0 }, 0, {    0,     0 }, { 255, 255, 255, 255 } } },
    { { {  24,  0, 0 }, 0, { 2048,     0 }, { 255, 255, 255, 255 } } },
    { { {  24, 96, 0 }, 0, { 2048, -4096 }, { 255, 255, 255, 255 } } },
    { { { -24, 96, 0 }, 0, {    0, -4096 }, { 255, 255, 255, 255 } } },
};

void TfUnusedFlame_Init(Actor* thisx, PlayState* play) {
    TfUnusedFlame* this = (TfUnusedFlame*)thisx;

    // Kokiri Forest, Zora's Domain and the Test Room hub don't preload OBJECT_FIRE in
    // their scene object lists, so a plain Object_GetIndex here returns -1 and the flame
    // never renders -- only the fallback point light shows up, which is too subtle to see
    // in daylight scenes. Request the slot ourselves via Object_Spawn, same pattern the
    // beta Kokiri actor uses; TfUnusedFlame_Update still gates `ready` on Object_IsLoaded.
    this->objBankIdx = Object_Spawn(&play->objectCtx, TF_UNUSED_FLAME_OBJECT);
    this->ready = 0;
    this->frames = 0;

    // Blue-ish flame to echo the En_Light big blue flame that used to mark this spot.
    Lights_PointNoGlowSetInfo(&this->light, 0, 0, 0, 80, 160, 255, 220);
    this->lightNode = LightContext_InsertLight(play, &play->lightCtx, &this->light);
}

void TfUnusedFlame_Destroy(Actor* thisx, PlayState* play) {
    TfUnusedFlame* this = (TfUnusedFlame*)thisx;
    LightContext_RemoveLight(play, &play->lightCtx, this->lightNode);
}

void TfUnusedFlame_Update(Actor* thisx, PlayState* play) {
    TfUnusedFlame* this = (TfUnusedFlame*)thisx;
    if (!this->ready) {
        if (this->objBankIdx >= 0 && Object_IsLoaded(&play->objectCtx, this->objBankIdx)) {
            this->ready = 1;
        }
    }
    this->frames++;

    // Keep the light pinned to the actor so it tracks any scripted re-position.
    this->light.params.point.x = (s16)this->actor.world.pos.x;
    this->light.params.point.y = (s16)this->actor.world.pos.y + 24;
    this->light.params.point.z = (s16)this->actor.world.pos.z;

    // Soft intensity flicker; same shape as En_Light's radius wiggle but gentler.
    s32 wiggle = (s32)(Math_SinS(this->frames * 0x1800) * 20.0f);
    this->light.params.point.radius = 220 + wiggle;
}

void TfUnusedFlame_Draw(Actor* thisx, PlayState* play) {
    TfUnusedFlame* this = (TfUnusedFlame*)thisx;
    if (!this->ready) {
        return;
    }

    // Bank segment 6 so Torch-published offsets inside object_fire resolve. The
    // texture pointers we hand to gDPLoadTextureBlock below are segmented into 6.
    void* segment = play->objectCtx.status[this->objBankIdx].segment;
    gSegments[6] = VIRTUAL_TO_PHYSICAL(segment);

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)segment);

    // Billboard toward camera, same math En_Light uses.
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateY(BINANG_TO_RAD((s16)(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000)), MTXMODE_APPLY);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
              G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 180, 220, 255, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 60, 160, 0);

    // 8-frame cycle at 7.5 Hz: frames >> 2 picks a texture index 0..7.
    s32 texIdx = (this->frames >> 2) & 7;
    gDPLoadTextureBlock(POLY_XLU_DISP++, sFireFrames[texIdx], G_IM_FMT_IA, G_IM_SIZ_8b, 32, 64, 0,
                        G_TX_WRAP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR, 5, 6, G_TX_NOLOD, G_TX_NOLOD);

    gSPVertex(POLY_XLU_DISP++, (uintptr_t)sFlameVtx, 4, 0);
    gSP2Triangles(POLY_XLU_DISP++, 0, 1, 2, 0, 0, 2, 3, 0);

    CLOSE_DISPS(play->state.gfxCtx);
}
