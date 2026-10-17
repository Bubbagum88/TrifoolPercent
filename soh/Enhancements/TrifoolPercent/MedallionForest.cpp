// Forest Medallion as a usable item -- beta ability restoration slice 2 of 6.
//
// Design note on scope. The beta leak (OoT Treasure Chest Audit / @MrCheeze_, 2021) says
// the Forest ("Wind") Medallion was Farore's Wind -- the warp spell. In vanilla OoT
// Farore's Wind is a separate inventory item (ITEM_FARORES_WIND) that occupies its own C
// slot and uses the magic bar. The restoration is: holding the Forest Medallion gives
// you Farore's Wind as a usable spell. Pattern matches MedallionFire.cpp (first slice of
// this series): CVar-gated OnPlayerUpdate hook, no engine-side changes required.
//
// The C-button-equip side of medallions -- the beta Sages'-Charm path that let you equip
// a medallion to a C button directly -- still needs engine edits (z_kaleido_scope +
// Player_ItemToItemAction) and lands in a separate engine-side PR. This overlay ships
// the gameplay equivalent: the Forest Medallion grants Farore's Wind in its own C slot
// alongside the medallion in the quest status screen.
//
// Inventory grant is idempotent: once the slot holds ITEM_FARORES_WIND it is not
// overwritten, so a player who later swaps in another item is not surprised.

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

#define CVAR_MEDALLION_FOREST_VALUE CVarGetInteger(CVAR_TRIFOOL_MEDALLION_FOREST, 0)

static bool PlayerHasForestMedallion() {
    return (gSaveContext.inventory.questItems & (1 << QUEST_MEDALLION_FOREST)) != 0;
}

// Called every player frame. Cheap: a bit test, a slot read and an early return. We fill
// the Farore's Wind inventory slot whenever the player holds the Forest Medallion, so the
// spell becomes equippable to a C button alongside the medallion's quest status entry.
// Only overwrites ITEM_NONE -- if the player has already placed something else in the
// slot (or has FW outright), we leave it alone.
static void MaybeGrantFaroresWind() {
    if (gPlayState == nullptr) {
        return;
    }
    if (!PlayerHasForestMedallion()) {
        return;
    }
    if (gSaveContext.inventory.items[SLOT_FARORES_WIND] == ITEM_NONE) {
        gSaveContext.inventory.items[SLOT_FARORES_WIND] = ITEM_FARORES_WIND;
    }
}

static void RegisterTrifoolMedallionForest() {
    COND_HOOK(OnPlayerUpdate, CVAR_MEDALLION_FOREST_VALUE, MaybeGrantFaroresWind);
}

static RegisterShipInitFunc medallionForestInitFunc(RegisterTrifoolMedallionForest,
                                                    { CVAR_TRIFOOL_MEDALLION_FOREST });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolMedallionForestWidget() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Forest Medallion grants Farore's Wind", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MEDALLION_FOREST)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Second slice of medallion-as-usable-item restoration. While the player holds the Forest "
            "Medallion, Farore's Wind is placed in its inventory slot (idempotent -- only fills an "
            "empty slot) so it becomes equippable to a C button. The beta Sages'-Charm path that "
            "equipped the medallion itself to C lives in SoH engine code and lands in a separate PR."));
}

static RegisterMenuInitFunc medallionForestMenuInitFunc(RegisterTrifoolMedallionForestWidget);
