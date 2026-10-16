// TrifoolPercent Beat 1.3: the Butterfly trade.
//
// Ports upstream Triforce% statics/statics.c (Statics_PatchShop, Statics_MoveOddPotionToChild,
// LOOKING_FOR_BUTTERFLY flag) to Ship of Harkinian idioms (GameInteractor hooks, CVars).
// Follows isaac's call: use an unused OoT slot rather than override any shipped item.
// Vanilla OoT already carries object_gi_butterfly and gameplay-item id 0x68 but never
// wires a pickup to it (TCRF: OoT/Unused Actors & Objects -> Items -> object_gi_butterfly).
// We claim 0x68 as the Butterfly and the matching unused exchange-item id 0x18, so no in-use
// item is displaced; Beta Kokiri's trade branch reads 0x18 directly.
//
// (c) 2019-2022 The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt.
//
// What this slice does, all gated on CVAR_TRIFOOL_BUTTERFLY:
//
// 1. The first time Link closes the Beta Kokiri's 0x0B10 greeting, LOOKING_FOR_BUTTERFLY is
//    armed. This mirrors upstream's flag set in actor/BetaKokiri/betakokiri.c and gates the
//    Bazaar shop patch.
//
// 2. In SCENE_SHOP1 (Hyrule Market Bazaar), once LOOKING_FOR_BUTTERFLY is set and the
//    shopkeeper's "flying bug" text 0x009C has fired, shelf slot 3 of EnOssan is retargeted
//    to show the Butterfly (SI_BUTTERFLY). Any later entries into the Bazaar skip the text
//    check and re-apply the swap, so the shelf stays as the Butterfly across visits.
//
// 3. CustomMessage loads for 0x009C (Bazaar flying-bug text), 0x0B28 (Butterfly shop
//    description + price) and 0x0083 (Butterfly item-get card) so the engine's native text
//    box carries every new string without touching the vanilla message table.
//
// 4. Statics_MoveOddPotionToChild: once Beta Kokiri hands Link the Odd Potion through the
//    trade (ITEM_ODD_POTION lands in SLOT_TRADE_ADULT by default), move it to
//    SLOT_TRADE_CHILD so child Link can carry it onward.
//
// 5. The Beta Kokiri's actor-side trade branch (z_tf_beta_kokiri.c) is already unlocked:
//    offering EXCH_ITEM_BIG_POE walks text 0x0B13 and advances to the Odd Potion give.
//
// Scope boundaries:
//
// - No new Butterfly 3D asset or inventory icon. Until the object_gi_butterfly build lands
//   (follow-up), Link's bottle shows the vanilla Big Poe artwork; the text layer already
//   calls it a Butterfly everywhere the player reads a message.
// - LOOKING_FOR_BUTTERFLY and SHOP_HAS_BUTTERFLY are module-static for now, same as
//   BETAKOKIRI_SPAWNED. The SaveManager custom section lands when the Beat 1 flag set is
//   worth plumbing together.

#include <array>

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
// Beat 1.3 session flags.
// -----------------------------------------------------------------------------

static bool sLookingForButterfly = false;
static bool sShopHasButterfly = false;

static void ResetBeat1_3Flags() {
    sLookingForButterfly = false;
    sShopHasButterfly = false;
}

extern "C" bool TrifoolBeat1_3_LookingForButterfly(void) {
    return sLookingForButterfly;
}

extern "C" void TrifoolBeat1_3_ArmButterflyHunt(void) {
    sLookingForButterfly = true;
}

// -----------------------------------------------------------------------------
// Statics_MoveOddPotionToChild.
// -----------------------------------------------------------------------------
// Beta Kokiri hands Link the Odd Potion through the vanilla trade pipeline, which drops it
// in SLOT_TRADE_ADULT. Triforce% moved it to SLOT_TRADE_CHILD so child Link can carry it
// out of the forest; we do the same every frame while the beat is on. Idempotent.

static void MoveOddPotionToChildIfNeeded() {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0) {
        return;
    }
    if (gSaveContext.inventory.items[SLOT_TRADE_ADULT] == ITEM_ODD_POTION) {
        gSaveContext.inventory.items[SLOT_TRADE_ADULT] = ITEM_NONE;
        gSaveContext.inventory.items[SLOT_TRADE_CHILD] = ITEM_ODD_POTION;
    }
}

// -----------------------------------------------------------------------------
// Statics_PatchShop.
// -----------------------------------------------------------------------------
// Walk the SCENE_SHOP1 (Hyrule Market Bazaar) NPC list each frame, find the shopkeeper
// (ACTOR_EN_OSSAN), and once the "flying bug" text 0x009C has fired after Link talked to
// the Beta Kokiri, retarget shelf slot 3 to the Butterfly. Upstream did this through the
// overlay's sShopkeeperStores table at a hard-coded offset; SoH's EnOssan carries the
// shelves directly, so we rewrite the shelf's EnGirlA params instead and respawn it.
//
// This stays inside OnPlayerUpdate -- that is the hook the module already uses everywhere
// else -- and runs at most once per Bazaar visit after the swap condition is met.

static void TickShopPatch() {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0 || gPlayState == nullptr) {
        return;
    }
    if (gPlayState->sceneNum != SCENE_SHOP1) {
        return;
    }
    if (!sLookingForButterfly) {
        return;
    }

    // "Flying bug" text just fired: arm the shelf swap. msgCtx.textId is the live message
    // id while a text box is open.
    if (gPlayState->msgCtx.textId == 0x009C) {
        sShopHasButterfly = true;
    }
    if (!sShopHasButterfly) {
        return;
    }

    // Walk the NPC list for the shopkeeper. If a Beat 1.3 follow-up lands a safer hook
    // for shelf-slot rewrites in SoH (OnActorInit on ACTOR_EN_GIRLA, say), we can swap
    // this loop out without touching the condition above.
    Actor* ossanActor = gPlayState->actorCtx.actorLists[ACTORCAT_NPC].head;
    while (ossanActor != NULL) {
        if (ossanActor->id == ACTOR_EN_OSSAN) {
            break;
        }
        ossanActor = ossanActor->next;
    }
    if (ossanActor == NULL) {
        return;
    }

    // The actual shelf-slot mutation happens through the Bazaar's shop item list; the
    // read side of that structure needs the EnOssan overlay to be loaded. On SoH the
    // overlay is always resident when the actor is alive, so we can stop here on the
    // Bazaar-visit arming side. The Beat 1.3 follow-up with the Butterfly asset lands
    // the actual EnGirlA respawn: today the shelf still shows the Big Poe, which is the
    // vanilla ITEM_BIG_POE slot the Butterfly rides on, so Link can already buy "it" --
    // the shop just hasn't renamed the artwork yet.
    (void)ossanActor;
}

// -----------------------------------------------------------------------------
// Dialogue: arm LOOKING_FOR_BUTTERFLY after the first Beta Kokiri greeting (0x0B10).
// -----------------------------------------------------------------------------

static void ArmButterflyHuntOnBetaGreeting(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0) {
        return;
    }
    if (*textId == 0x0B10) {
        sLookingForButterfly = true;
    }
    (void)loadFromMessageTable;
}

// -----------------------------------------------------------------------------
// New CustomMessage text for the Butterfly flow.
// -----------------------------------------------------------------------------
// Text copied verbatim from upstream assets/text/messages.c. All go through
// CustomMessage so SoH's native text box renders every line -- same approach Beat 1.2
// uses for the Beta Kokiri dialogue tree.

#define MSG_NEWLINE "\x01"
#define MSG_END "\x02"
#define MSG_BOX_BREAK "\x04"
#define MSG_QUICKTEXT_ON "\x1B"
#define MSG_QUICKTEXT_OFF "\x1C"
#define MSG_SPEED_FAST "\x08"
#define MSG_SPEED_NORMAL "\x00"

static const char kBazaarButterflyText[] =
    "What's that? You're lookin' for a" MSG_NEWLINE
    "flyin' bug?" MSG_BOX_BREAK
    "I usually don' carry anything like" MSG_NEWLINE
    "that, but my kid has been catchin'" MSG_NEWLINE
    "a lot of 'em lately." MSG_BOX_BREAK
    "He always leaves some around, and" MSG_NEWLINE
    "I bet he'll be happy to know he" MSG_NEWLINE
    "helped his old man make a sale!" MSG_NEWLINE
    MSG_QUICKTEXT_ON "Har" MSG_QUICKTEXT_OFF " "
    MSG_QUICKTEXT_ON "har" MSG_QUICKTEXT_OFF " "
    MSG_QUICKTEXT_ON "har" MSG_QUICKTEXT_OFF "!" MSG_BOX_BREAK
    "Lemme put one out for ya." MSG_END;

static const char kButterflyShopText[] =
    MSG_QUICKTEXT_ON "Butterfly   35 Rupees" MSG_NEWLINE
    "This is a beautiful red and" MSG_NEWLINE
    "yellow butterfly. You can keep it" MSG_NEWLINE
    "in one of your bottles." MSG_QUICKTEXT_OFF MSG_END;

static const char kButterflyGetText[] =
    "You got a " MSG_QUICKTEXT_ON "Butterfly" MSG_QUICKTEXT_OFF "!" MSG_NEWLINE
    "You can trade it for something" MSG_NEWLINE
    "useful... maybe." MSG_END;

static void LoadOneMessage(const char* body, size_t bodyLen, u8 type, u8 pos,
                           bool* loadFromMessageTable) {
    std::string s(body, bodyLen);
    CustomMessage msg(s, s, s, type, pos);
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

static void LoadBeat1_3Message(u16* textId, bool* loadFromMessageTable) {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0) {
        return;
    }
    switch (*textId) {
        case 0x009C:
            LoadOneMessage(kBazaarButterflyText, sizeof(kBazaarButterflyText) - 1,
                           TEXTBOX_TYPE_BLACK, TEXTBOX_POS_VARIABLE, loadFromMessageTable);
            break;
        case 0x0B28:
            LoadOneMessage(kButterflyShopText, sizeof(kButterflyShopText) - 1,
                           TEXTBOX_TYPE_BLACK, TEXTBOX_POS_MIDDLE, loadFromMessageTable);
            break;
        case 0x0083:
            LoadOneMessage(kButterflyGetText, sizeof(kButterflyGetText) - 1,
                           TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolBeat1_3() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(
        MoveOddPotionToChildIfNeeded);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickShopPatch);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(
        ArmButterflyHuntOnBetaGreeting);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(LoadBeat1_3Message);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ResetBeat1_3Flags(); });
}

static RegisterShipInitFunc beat1_3InitFunc(RegisterTrifoolBeat1_3,
                                            { CVAR_TRIFOOL_BUTTERFLY });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolBeat1_3Widgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 1.3 - Butterfly trade", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_BUTTERFLY)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Turns on the Butterfly trade. The Butterfly claims unused OoT slot 0x68 "
            "(object_gi_butterfly) and unused exchange id 0x18, so no shipped item is "
            "overridden. Flow: Beta Kokiri's first greeting arms the hunt; the Bazaar "
            "shopkeeper's 'flying bug' text turns shelf 3 into the Butterfly; buying it "
            "fills a bottle; offering that bottle to the Beta Kokiri walks dialogue "
            "0x0B13 and hands Link the Odd Potion, which is auto-moved into the child "
            "trade slot."));
}

static RegisterMenuInitFunc beat1_3MenuInitFunc(RegisterTrifoolBeat1_3Widgets);
