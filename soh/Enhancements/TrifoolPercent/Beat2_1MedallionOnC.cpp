// TrifoolPercent Beat 2.1: Sages' Charm medallion-on-C enabler + beta item action.
//
// Lands patches #1, #5 and #7 from the Triforce% statics/interface.c breakdown:
//
//   #1  Statics_HandleEquipMedallionsToC  -> TrifoolBeat_UpdateQuestEquipHook
//   #5  Patched_MagicBarSetup Y1/Loads    -> TrifoolBeat_MagicBarYOffset
//   #7  Patched_Player_ItemToItemAction   -> TrifoolBeat_MapMedallionItemAction
//
// Each of the three symbols is a weak default in Shipwright (see
// patches/shipwright/*.patch); when this module is compiled in, the strong
// definitions below take over. All three are gated on CVAR_TRIFOOL_MEDALLION_ON_C
// plus TrifoolBeat_HasSagesCharm() from Beat 2.0, so a build with the module in
// but the toggle off is pixel- and behaviour-identical to vanilla.
//
// Scope boundaries. The upstream behaviour we match in this slice:
//
//   * While the Charm is held and the Quest-Status subscreen is open, pressing
//     a C-button on a medallion the player owns equips that medallion to the
//     corresponding C-button, reusing SoH's native KaleidoScope_SetupItemEquip
//     animation + save-write pipeline.
//   * While the Charm is held, the magic bar sits 3px lower, reserving room
//     for the medallion-count display the follow-up Beat 2.2 slice will add.
//   * When the Fire Medallion is active on the B/C slot the player action is
//     PLAYER_IA_DINS_FIRE, so pressing the C-button casts Din's Fire.
//
// Explicit non-goals (follow-up slices):
//
//   * The 24x24 medallion-on-C icon. SoH's icon segment DMAs out of
//     icon_item_static at an item-id offset; medallion items 0x66..0x6B already
//     have 32x32 icons there, so the C-button renders the vanilla medallion
//     face directly. The upstream "smaller" icon is cosmetic polish, split.
//   * The Fire-Medallion-casts-Din's-Fire-even-with-zero-magic rider. Vanilla
//     Din's Fire enforces its own magic cost; keeping that is intentional.
//   * The medallion-count display proper. Lives in Beat 2.2 and consumes the
//     3px the magic-bar shift reserves.
//   * Spirit (Soul) / Shadow (Darkness) beta abilities. Split per-medallion.

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"
#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

extern "C" {
#include "TrifoolPercent.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "z64save.h"
#include "overlays/misc/ovl_kaleido_scope/z_kaleido_scope.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

// -----------------------------------------------------------------------------
// Patch #1: Quest-page C-equip hook.
// -----------------------------------------------------------------------------

// Called from the top of KaleidoScope_DrawQuestStatus each pause-frame. Replaces
// the module-side "pause-frame" state that upstream kept in interface.c; we live
// on the kaleido tick directly since SoH already dispatches us once per frame.
extern "C" void TrifoolBeat_UpdateQuestEquipHook(PlayState* play) {
    if (CVarGetInteger(CVAR_TRIFOOL_MEDALLION_ON_C, 0) == 0) {
        return;
    }
    if (!TrifoolBeat_HasSagesCharm()) {
        return;
    }
    if (play == nullptr) {
        return;
    }
    PauseContext* pauseCtx = &play->pauseCtx;
    // state 6 is "interactive menu open"; unk_1E4 == 0 is "no equip animation
    // already running"; cursorSpecialPos == 0 is "cursor on a tile, not on a
    // page-arrow". Mirrors the state gate upstream used.
    if (pauseCtx->state != 6 || pauseCtx->unk_1E4 != 0 || pauseCtx->cursorSpecialPos != 0) {
        return;
    }
    if (pauseCtx->debugState != 0 || pauseCtx->pageIndex != PAUSE_QUEST) {
        return;
    }
    // Medallions sit at cursorPoint 0..5; cursorItem[PAUSE_QUEST] is their item
    // id (0x66..0x6B = ITEM_MEDALLION_FOREST..LIGHT).
    s16 cursorPoint = pauseCtx->cursorPoint[PAUSE_QUEST];
    if (cursorPoint < 0 || cursorPoint >= 6) {
        return;
    }
    u16 item = pauseCtx->cursorItem[PAUSE_QUEST];
    if (item < ITEM_MEDALLION_FOREST || item > ITEM_MEDALLION_LIGHT) {
        return;
    }
    // Must actually own the medallion.
    if (!CHECK_QUEST_ITEM(cursorPoint)) {
        return;
    }
    Input* input = &play->state.input[0];
    if (!(CHECK_BTN_ALL(input->press.button, BTN_CLEFT) ||
          CHECK_BTN_ALL(input->press.button, BTN_CDOWN) ||
          CHECK_BTN_ALL(input->press.button, BTN_CRIGHT))) {
        return;
    }
    // Cursor pixel position. Mirror upstream: anim starts at the cursor.
    s16 animX = pauseCtx->cursorVtx[0].v.ob[0] * 10;
    s16 animY = pauseCtx->cursorVtx[0].v.ob[1] * 10;
    // Slot is a logical index SoH stores in cButtonSlots[cBtn]. Medallions are
    // not in inventory.items[], so use SLOT_NONE: player ammo lookups short-
    // circuit on it, and buttonItems[cBtn] still carries the medallion id for
    // the C-button icon and for Player_ItemToItemAction's dispatch (patch #7).
    KaleidoScope_SetupItemEquip(play, item, SLOT_NONE, animX, animY);
}

// -----------------------------------------------------------------------------
// Patch #5: magic bar Y offset.
// -----------------------------------------------------------------------------

extern "C" s16 TrifoolBeat_MagicBarYOffset(void) {
    if (CVarGetInteger(CVAR_TRIFOOL_MEDALLION_ON_C, 0) == 0) {
        return 0;
    }
    if (!TrifoolBeat_HasSagesCharm()) {
        return 0;
    }
    return 3;
}

// -----------------------------------------------------------------------------
// Patch #7: Fire Medallion on a C-button acts as Din's Fire.
// -----------------------------------------------------------------------------

extern "C" s8 TrifoolBeat_MapMedallionItemAction(s32 item, s8 fallback) {
    if (CVarGetInteger(CVAR_TRIFOOL_MEDALLION_ON_C, 0) == 0) {
        return fallback;
    }
    if (!TrifoolBeat_HasSagesCharm()) {
        return fallback;
    }
    // Only Fire Medallion gets a mapped action in this slice. The other five
    // medallions keep the vanilla fallback (which is PLAYER_IA_NONE for them)
    // and ride on their own ability slices (MedallionWater / MedallionLight /
    // MedallionForest already live in this module; Shadow and Spirit are TBD).
    if (item == ITEM_MEDALLION_FIRE) {
        return PLAYER_IA_DINS_FIRE;
    }
    return fallback;
}

// -----------------------------------------------------------------------------
// Menu widget.
// -----------------------------------------------------------------------------

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolBeat2_1Widgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 2.1 - Medallions on C (Sages' Charm)",
                                WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MEDALLION_ON_C)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "While the Sages' Charm (Beat 2.0) is held: pressing a C-button on a "
            "medallion in the Quest Status subscreen equips it to that C-button, "
            "and Fire Medallion-on-C fires Din's Fire. Also shifts the magic bar "
            "down 3px to reserve space for the Beat 2.2 medallion-count display. "
            "Requires the three Shipwright engine patches under patches/shipwright/ "
            "to be applied by tools/build_soh.sh; weak-symbol defaults keep a "
            "patched engine binary with this toggle off identical to vanilla."));
}

static RegisterMenuInitFunc beat2_1MenuInitFunc(RegisterTrifoolBeat2_1Widgets);
