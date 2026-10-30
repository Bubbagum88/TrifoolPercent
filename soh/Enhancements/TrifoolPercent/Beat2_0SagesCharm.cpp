// TrifoolPercent Beat 2.0: the Sages' Charm (also called the "Oracle Amulet" in the
// Triforce% showcase VOD; upstream in-world text is "Sages' Charm").
//
// Adapts upstream Triforce% statics/interface.c + statics/statics.c, (C) 2019-2022
// The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt. The upstream
// pieces this file reaches for:
//   - interface.c:128  Statics_HandleEquipMedallionsToC    (medallions on C-buttons)
//   - interface.c:237  HasSagesCharm                       (predicate for every patch)
//   - interface.c:306  gDrawItemTable[GID_SAGES_CHARM]     (model DL wiring)
//   - interface.c:320  relocGetItemTable[GI_SAGES_CHARM-1] (object slot binding)
//   - interface.c:241  Patched_MagicBarSetup/Y1/Loads      (magic bar visual)
//   - statics.c:265    SAGES_CHARM_VAR |= SAGES_CHARM_BIT  (grant hook)
//
// Scope of this slice (foundation only):
//
// 1. Session flag: sHasSagesCharm, exposed through TrifoolBeat_HasSagesCharm /
//    TrifoolBeat_GrantSagesCharm so follow-up slices (medallion-on-C, Din's Fire on C,
//    magic bar visual) can gate on it without each one re-defining the predicate.
//
// 2. Item id + text id: TRIFOOL_ITEM_SAGES_CHARM 0x69 (next unused OoT quest-status
//    slot after Butterfly's 0x68) and TRIFOOL_TEXT_SAGES_CHARM_GET 0x0084 (next free
//    vanilla text id after Butterfly's 0x0083). Both documented in TrifoolPercent.h.
//
// 3. CustomMessage item-get card loaded through OnOpenText, same pattern as Butterfly
//    (Beat 1.3): "You got the Sages' Charm!" plus the beta-ability tagline. The
//    engine renders this through its native TEXTBOX_TYPE_BLUE item-get box with the
//    item-get fanfare, so once a chest or a hand-off actor calls Message_StartTextbox
//    with TRIFOOL_TEXT_SAGES_CHARM_GET the player sees the vanilla receive-item card
//    (minus the 3D model preview, which rides on the ItemTableManager follow-up).
//
// 4. Menu widget: toggle beat on + a one-shot "Grant Sages' Charm (debug)" button that
//    sets the flag and fires the item-get textbox. Lets testing of the follow-up
//    medallion / Din's Fire / magic-bar slices proceed without the Running Man boss.
//
// 5. Reset-on-save-load so the flag doesn't bleed between files.
//
// Explicit NON-goals of this slice (each gets its own PR):
//
// - Medallion-on-C-button pause-screen enabler. Upstream patches z_kaleido_scope and
//   sSubscreenButtonStates; in SoH this is a REGISTER_VB_SHOULD on the kaleido C-button
//   gate or a direct kaleido edit. Needs the fork's kaleido patch to compile, so split.
// - Fire Medallion on C performs Din's Fire. Upstream patches z_player's
//   Player_ItemToItemAction; in SoH this is a Player VB hook.
// - Magic-bar visual patches. Visual-only, defer until the above two are proven.
// - 24px C-button icon. Needs an icon asset (upstream's new 24x24 ia8). The breakdown
//   calls it "new 24px icon"; without the asset this is a drop-in file wire-up.
// - 3D GetItemEntry registration via ItemTableManager + object_gi_sagescharm.o2r.
//   The model .c + display list symbol gGiSagesCharmDL are already exported in
//   assets/model/SagesCharm/, same state as Butterfly: lands when the o2r build adds
//   the SagesCharm object. See TrifoolPercent.h on the storage decision.
// - Running Man boss handoff. BossRunningMan is not yet ported to a SoH actor; when it
//   is, its "deliver reward" state calls TrifoolBeat_GrantSagesCharm and starts text
//   TRIFOOL_TEXT_SAGES_CHARM_GET through the vanilla En_Box pipeline.
//
// All of the above are listed in docs/items/sages-charm.md.

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/custom-message/CustomMessageManager.h"
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
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

// -----------------------------------------------------------------------------
// Beat 2.0 session flag + exports.
// -----------------------------------------------------------------------------

static bool sHasSagesCharm = false;

// Debug one-shot: when the menu button is pressed, we grant and open the item-get
// textbox next OnPlayerUpdate (safer than firing textboxes directly from the UI hook,
// which can run while no PlayState is active).
static bool sPendingGrantTextbox = false;

static void ResetBeat2_0Flags() {
    sHasSagesCharm = false;
    sPendingGrantTextbox = false;
}

extern "C" bool TrifoolBeat_HasSagesCharm(void) {
    return sHasSagesCharm;
}

extern "C" void TrifoolBeat_GrantSagesCharm(void) {
    sHasSagesCharm = true;
}

// -----------------------------------------------------------------------------
// CustomMessage item-get card.
// -----------------------------------------------------------------------------
// Text copied from Triforce% assets/text/messages.c (the "You got the Sages' Charm!"
// box) with the beta-ability tagline we reconstructed from the showcase. All goes
// through CustomMessage so SoH's native text box renders every line.

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_BOX_BREAK "\x04"
#define MSG_QUICKTEXT_ON "\x1B"
#define MSG_QUICKTEXT_OFF "\x1C"

static const char kSagesCharmGetText[] =
    "You got the " MSG_QUICKTEXT_ON "Sages' Charm" MSG_QUICKTEXT_OFF "!" MSG_NEWLINE
    "You can now equip Medallions to the" MSG_NEWLINE
    "C-buttons. The Fire Medallion acts" MSG_NEWLINE
    "as " MSG_QUICKTEXT_ON "Din's Fire" MSG_QUICKTEXT_OFF "." MSG_END;

static void LoadOneMessage(const char* body, size_t bodyLen, u8 type, u8 pos,
                           bool* loadFromMessageTable) {
    std::string s(body, bodyLen);
    CustomMessage msg(s, s, s, static_cast<TextBoxType>(type), static_cast<TextBoxPosition>(pos));
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

static void LoadBeat2_0Message(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_SAGES_CHARM, 0) == 0) {
        return;
    }
    if (*textId == TRIFOOL_TEXT_SAGES_CHARM_GET) {
        LoadOneMessage(kSagesCharmGetText, sizeof(kSagesCharmGetText) - 1,
                       TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
    }
}

// -----------------------------------------------------------------------------
// Debug grant hook.
// -----------------------------------------------------------------------------

static void TickDebugGrant() {
    // Prefer the menu-driven pulse CVar: a tick becomes a one-shot grant + textbox.
    if (CVarGetInteger(CVAR_TRIFOOL_SAGES_CHARM, 0) == 0) {
        return;
    }
    if (CVarGetInteger(CVAR_TRIFOOL_SAGES_CHARM_GRANT_PULSE, 0) != 0) {
        CVarSetInteger(CVAR_TRIFOOL_SAGES_CHARM_GRANT_PULSE, 0);
        CVarSave();
        sPendingGrantTextbox = true;
    }
    if (!sPendingGrantTextbox) {
        return;
    }
    if (gPlayState == nullptr) {
        return;
    }
    // Hold the request until we're not already inside a textbox, so we don't stomp
    // another message mid-render.
    if (gPlayState->msgCtx.msgMode != MSGMODE_NONE) {
        return;
    }
    sPendingGrantTextbox = false;
    sHasSagesCharm = true;
    Message_StartTextbox(gPlayState, TRIFOOL_TEXT_SAGES_CHARM_GET, NULL);
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolBeat2_0() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(LoadBeat2_0Message);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickDebugGrant);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ResetBeat2_0Flags(); });
}

static RegisterShipInitFunc beat2_0InitFunc(RegisterTrifoolBeat2_0,
                                            { CVAR_TRIFOOL_SAGES_CHARM });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterBeat2_0Widgets() {
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 2.0 - Sages' Charm (Oracle Amulet)",
                                WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_SAGES_CHARM)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Lays the foundation for the Sages' Charm (Oracle Amulet). This slice "
            "wires only the session flag, item id (0x69, unused in vanilla), text id "
            "(0x0084) and the item-get textbox. The medallion-on-C, Din's Fire on C "
            "and magic-bar follow-ups each ship as their own slice. Use the debug "
            "grant button below to arm the flag and preview the item-get card without "
            "the Running Man boss."));
    SohGui::mSohMenu->AddWidget(path,
                                "Grant Sages' Charm now (debug)",
                                WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TrifoolPercent.SagesCharmGrantPulse"))
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Tick to grant the Sages' Charm and open the item-get textbox once. The "
            "box opens on the next frame the player is not already in a textbox. "
            "Requires the Beat 2.0 toggle above to be on. Clears itself after firing."));
}

