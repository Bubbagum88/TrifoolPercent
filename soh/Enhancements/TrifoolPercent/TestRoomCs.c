/*
 * Prototype cutscene for the TrifoolPercent Test Room (SCENE_SYOTES, map select "118:Syotes").
 *
 * Hand-written against soh/include/z64cutscene_commands.h to prove the authoring + trigger
 * pipeline we documented in docs/cutscene-plan.md. Everything the real story-beat cutscenes
 * need is here in miniature: a text list routed through CustomMessageManager, a camera-eye
 * list, a matching camera-focus list, and nothing else. No actors, no object loads, no music
 * - this is the ground floor the Fast64-authored cutscenes will build on.
 *
 * Camera path: 8-keyframe orbit around the hex room's centre at radius 500 OoT units and
 * height 150, looking slightly above the floor at (0, 20, 0). Durations are generous (300
 * frames, ~10 seconds) so the pan reads at normal playback speed.
 *
 * Triggered from TestRoom.cpp on scene-spawn when CVAR_TRIFOOL_TEST_ROOM_PROTOTYPE_CS is on.
 */

#include "TrifoolPercent.h"
#include "z64cutscene_commands.h"

// clang-format off
CutsceneData gTrifoolTestRoomPrototypeCs[] = {
    CS_BEGIN_CUTSCENE(3, 300),
    CS_TEXT_LIST(3),
        CS_TEXT_DISPLAY_TEXTBOX(0x1610, 20, 90, 0x0000, 0x0000, 0x0000),
        CS_TEXT_DISPLAY_TEXTBOX(0x1611, 100, 180, 0x0000, 0x0000, 0x0000),
        CS_TEXT_DISPLAY_TEXTBOX(0x1612, 190, 270, 0x0000, 0x0000, 0x0000),
    CS_CAM_POS_LIST(0, 300),
        CS_CAM_POS(0, 0, 0, 60.0f,  500, 150,    0, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  354, 150, -354, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,    0, 150, -500, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -354, 150, -354, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -500, 150,    0, 0),
        CS_CAM_POS(0, 0, 0, 60.0f, -354, 150,  354, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,    0, 150,  500, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  354, 150,  354, 0),
        CS_CAM_POS(0, 0, 0, 60.0f,  500, 150,    0, 0),
        CS_CAM_POS(-1, 0, 0, 60.0f, 500, 150,    0, 0),
    CS_CAM_FOCUS_POINT_LIST(0, 320),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(0, 0, 20, 60.0f, 0, 20, 0, 0),
        CS_CAM_FOCUS_POINT(-1, 0, 20, 60.0f, 0, 20, 0, 0),
    CS_END(),
};
// clang-format on

// Text ids reserved for the prototype cutscene (parallel to the sign block at 0x1600..0x1605).
// TestRoom.cpp routes these through CustomMessageManager alongside the sign messages.
const u16 gTrifoolTestRoomPrototypeCsTextBase = 0x1610;
const u8 gTrifoolTestRoomPrototypeCsTextCount = 3;
