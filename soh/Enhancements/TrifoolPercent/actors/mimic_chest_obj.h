/*
 * Mimic Chest object data, ported verbatim from Triforce% actor/MimicChest/MimicChestObj.h.
 * Arrays live in mimic_chest_obj.c; animations in mimic_chest_idle_anim.c and
 * mimic_chest_attack_anim.c. Everything compiles straight into the SoH module as plain
 * C data (no Torch object bank), so z_tf_mimic_chest.c references the symbols by pointer.
 */

#ifndef TRIFOOL_MIMIC_CHEST_OBJ_H
#define TRIFOOL_MIMIC_CHEST_OBJ_H

#include <libultraship/libultra.h>
#include "global.h"

extern FlexSkeletonHeader MimicChestObj;
extern AnimationHeader MimicChestObjIdleAnim;
extern AnimationHeader MimicChestObjAttackAnim;

#define MIMICCHESTOBJ_ROOT_POS_LIMB 0
#define MIMICCHESTOBJ_ROOT_ROT_LIMB 1
#define MIMICCHESTOBJ_LID_LIMB      2
#define MIMICCHESTOBJ_NUM_LIMBS     3

#endif
