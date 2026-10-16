/*
 * Unicorn Fountain entry cutscene.
 *
 * Hand-written against soh/include/z64cutscene_commands.h in the same style as TestRoomCs.c, which
 * proves the authoring path end-to-end (see docs/cutscene-plan.md Phase C). This file is written
 * directly in the macro output a Fast64 export of scenes/UnicornFountain/UnicornFountain.blend
 * would produce; when the .blend is re-exported with its own cutscene scene, it replaces this
 * CutsceneData[] (ids and the UnicornFountain.cpp trigger stay).
 *
 * Shape: a 240-frame camera pan arriving in the fountain. 8-keyframe orbit around the room
 * centre at radius 400 OoT units, rising from height 60 (floor-level approach) to 180 (over the
 * fairy's rise point), focusing on the fairy spawn point (0, 80, 0). No textboxes, no music -
 * the room's title card and the Beta Great Fairy's own dialogue carry the content; the cutscene
 * is purely the arrival camera. The Beta Great Fairy actor holds in state 0 while the cutscene
 * runs (z_tf_beta_great_fairy.c pauses its auto-rise counter on csCtx.state != CS_STATE_IDLE),
 * so her rise reads as a response to the camera landing on her, not a parallel timer.
 */

#include "TrifoolPercent.h"
#include "z64cutscene_commands.h"

// clang-format off
CutsceneData gTrifoolUnicornFountainEntryCs[] = {
    CS_BEGIN_CUTSCENE(2, 240),
    CS_CAM_POS_LIST(0, 240),
        CS_CAM_POS(0, 0, 0, 60.0f,  400,  60,    0, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  283,  75, -283, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,    0,  95, -400, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -283, 120, -283, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -400, 140,    0, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -283, 155,  283, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,    0, 170,  400, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  283, 180,  283, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  400, 180,    0, 0),
        CS_CAM_POS(-1, 0, 0, 60.0f, 400, 180,    0, 0),
    CS_CAM_FOCUS_POINT_LIST(0, 260),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0,  80, 0, 0),
        CS_CAM_FOCUS_POINT(-1, 0, 20, 60.0f, 0, 80, 0, 0),
    CS_END(),
};
// clang-format on
