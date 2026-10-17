// Light Medallion as a usable item -- beta ability restoration slice 4 of 6.
//
// Design note on scope. The beta leak (OoT Treasure Chest Audit / @MrCheeze_, 2021)
// names the Light Medallion as "illuminates darkness." The direct implementation --
// overriding scene ambient light / fog when the medallion is held -- would reach into
// gPlayState->envCtx fields that SoH may skin differently per fork, so it lives in a
// separate engine-side PR. This slice ships the gameplay proxy consistent with the
// earlier medallion slices: the medallion grants its elemental weapon counterpart. Light
// Arrows are the vanilla sacred-light projectile; they are the canonical "light pierces
// darkness" item in OoT.

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

#define CVAR_MEDALLION_LIGHT_VALUE CVarGetInteger(CVAR_TRIFOOL_MEDALLION_LIGHT, 0)

static bool PlayerHasLightMedallion() {
    return (gSaveContext.inventory.questItems & (1 << QUEST_MEDALLION_LIGHT)) != 0;
}

static void MaybeGrantLightArrows() {
    if (gPlayState == nullptr) {
        return;
    }
    if (!PlayerHasLightMedallion()) {
        return;
    }
    if (gSaveContext.inventory.items[SLOT_ARROW_LIGHT] == ITEM_NONE) {
        gSaveContext.inventory.items[SLOT_ARROW_LIGHT] = ITEM_ARROW_LIGHT;
    }
}

static void RegisterTrifoolMedallionLight() {
    COND_HOOK(OnPlayerUpdate, CVAR_MEDALLION_LIGHT_VALUE, MaybeGrantLightArrows);
}

static RegisterShipInitFunc medallionLightInitFunc(RegisterTrifoolMedallionLight,
                                                   { CVAR_TRIFOOL_MEDALLION_LIGHT });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolMedallionLightWidget() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Light Medallion grants Light Arrows", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MEDALLION_LIGHT)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Fourth slice of medallion-as-usable-item restoration. While the player holds the Light "
            "Medallion, Light Arrows are placed in their inventory slot (idempotent -- only fills an "
            "empty slot) so they become equippable to a C button. The beta 'passive ambient "
            "illumination' behaviour needs engine-side envCtx edits and lands in a separate PR."));
}

static RegisterMenuInitFunc medallionLightMenuInitFunc(RegisterTrifoolMedallionLightWidget);
