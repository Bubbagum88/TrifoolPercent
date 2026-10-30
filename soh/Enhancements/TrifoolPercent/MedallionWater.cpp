// Water Medallion as a usable item -- beta ability restoration slice 3 of 6.
//
// Design note on scope. The beta leak (OoT Treasure Chest Audit / @MrCheeze_, 2021)
// names the Water ("Ice") Medallion as "puts out fire." The direct implementation --
// hooking Obj_Syokudai torch actors to clear their lit state -- lives in SoH engine
// code that is not in this overlay. This slice ships the gameplay proxy consistent with
// the Fire (#52) and Forest (#53) pattern: the medallion grants its elemental weapon
// counterpart in the C-slot. Ice Arrows are the vanilla cold projectile and extinguish
// torches when fired at them, which lines up with the beta description.
//
// The exact "walk into a lit torch, it goes out passively" behaviour will land once the
// engine-side actor-hook PR is in. Same scope rules as the earlier slices.

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
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define CVAR_MEDALLION_WATER_VALUE CVarGetInteger(CVAR_TRIFOOL_MEDALLION_WATER, 0)

static bool PlayerHasWaterMedallion() {
    return (gSaveContext.inventory.questItems & (1 << QUEST_MEDALLION_WATER)) != 0;
}

static void MaybeGrantIceArrows() {
    if (gPlayState == nullptr) {
        return;
    }
    if (!PlayerHasWaterMedallion()) {
        return;
    }
    if (gSaveContext.inventory.items[SLOT_ARROW_ICE] == ITEM_NONE) {
        gSaveContext.inventory.items[SLOT_ARROW_ICE] = ITEM_ARROW_ICE;
    }
}

static void RegisterTrifoolMedallionWater() {
    COND_HOOK(OnPlayerUpdate, CVAR_MEDALLION_WATER_VALUE, MaybeGrantIceArrows);
}

static RegisterShipInitFunc medallionWaterInitFunc(RegisterTrifoolMedallionWater,
                                                   { CVAR_TRIFOOL_MEDALLION_WATER });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterMedallionWaterWidget() {
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Water Medallion grants Ice Arrows", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MEDALLION_WATER)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Third slice of medallion-as-usable-item restoration. While the player holds the Water "
            "Medallion, Ice Arrows are placed in their inventory slot (idempotent -- only fills an "
            "empty slot) so they become equippable to a C button. The beta 'walk into a torch, it "
            "goes out' behaviour needs an engine-side actor hook on Obj_Syokudai and lands in a "
            "separate PR."));
}

