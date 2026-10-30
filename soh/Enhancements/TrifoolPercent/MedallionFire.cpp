// Fire Medallion as a usable item -- beta ability restoration slice 1 of 6.
//
// Design note on scope. Triforce% re-enabled the beta behaviour where each medallion is
// usable (not just a quest icon) once the Sages' Charm let you equip medallions to C
// buttons. The Fire Medallion in particular became Din's Fire -- same spell, but with the
// extra beta property that it can melt Zora's Domain. See the beta leak summary (OoT
// Treasure Chest Audit / @MrCheeze_, 2021) and docs/references/triforcepercent/code-breakdown.md
// section 2 ("Sages' Charm" bullets) and section 3.1 ("Zora's Domain ice melting").
//
// The C-button-equip path and the "cast Din's Fire from the medallion slot" link both
// live in SoH engine code (z_kaleido_scope, Player_ItemToItemAction). Those edits land in
// a separate engine-side PR; this overlay module ships the gameplay effect that is
// achievable from an enhancement hook:
//
//   When the player holds the Fire Medallion and the CVar is on, Zora's Domain is
//   considered thawed -- i.e. the vanilla EVENTCHKINF_KING_ZORA_MOVED event flag is
//   set, so the engine picks the normal setup instead of the frozen setup on the next
//   scene load.
//
// This lines up with Isaac's framing ("the only difference will be that it will be able
// to melt Zora's domain") without pretending the C-button side is finished. A follow-up
// slice will tie the melt to a Din's Fire cast specifically (via a VB_ hook on the magic
// spell actor) once the C-button equip is in place.

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

#define CVAR_MEDALLION_FIRE_VALUE CVarGetInteger(CVAR_TRIFOOL_MEDALLION_FIRE, 0)

static bool PlayerHasFireMedallion() {
    return (gSaveContext.inventory.questItems & (1 << QUEST_MEDALLION_FIRE)) != 0;
}

// Called every player frame. Cheap: two ints and an early return. We set the UNFROZE
// flag whenever the player holds the Fire Medallion so that the next entry to Zora's
// Domain picks setup 0 (normal) instead of setup 2 (frozen), without the player having
// to visit any other scene first. Idempotent: SET_EVENTCHKINF is a bit-or.
static void MaybeThawZorasDomain() {
    if (gPlayState == nullptr) {
        return;
    }
    if (!PlayerHasFireMedallion()) {
        return;
    }
    // EVENTCHKINF_KING_ZORA_MOVED (0x33) is the vanilla "King Zora has moved aside after
    // thawing" flag. Setting it makes Zora's Domain pick its normal scene setup instead
    // of the frozen one on the next scene load.
    if (!GET_EVENTCHKINF(EVENTCHKINF_KING_ZORA_MOVED)) {
        SET_EVENTCHKINF(EVENTCHKINF_KING_ZORA_MOVED);
    }
}

static void RegisterTrifoolMedallionFire() {
    COND_HOOK(OnPlayerUpdate, CVAR_MEDALLION_FIRE_VALUE, MaybeThawZorasDomain);
}

static RegisterShipInitFunc medallionFireInitFunc(RegisterTrifoolMedallionFire,
                                                  std::set<std::string>{ CVAR_TRIFOOL_MEDALLION_FIRE });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterMedallionFireWidget() {
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Fire Medallion melts Zora's Domain", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MEDALLION_FIRE)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "First slice of medallion-as-usable-item restoration. While the player holds the Fire "
            "Medallion, Zora's Domain is treated as thawed (sets EVENTCHKINF_KING_ZORA_MOVED), so "
            "the engine picks the normal scene setup instead of the frozen one on the next entry. "
            "The C-button-equip side and the 'cast Din's Fire to melt' trigger live in SoH engine "
            "code and land in a separate PR."));
}

