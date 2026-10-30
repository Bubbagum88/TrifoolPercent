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
// 2. In SCENE_BAZAAR (Hyrule Market Bazaar), once LOOKING_FOR_BUTTERFLY is set and the
//    shopkeeper's "flying bug" text 0x009C has fired, shelf slot 3 of EnOssan is retargeted
//    to SI_BIG_POE and respawned via Actor_Spawn. (A dedicated SI_BUTTERFLY shelf index
//    would need SoH-source edits to z_en_girla.{c,h}; from the mod we ride SI_BIG_POE and
//    rely on the GI_BIG_POE GetItemEntry override + icon + dialogue to re-skin it as the
//    Butterfly end-to-end.) Any later entry into the Bazaar skips the text check and
//    re-applies the swap only when shelf 3 is not already SI_BIG_POE, so the shelf stays
//    stable across visits.
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
// 6. The HUD icon on any C button holding ITEM_BIG_POE is re-blit each frame with the
//    butterfly texture (ButterflyIcon.inc.h), mirroring upstream Triforce%
//    scripts/statics/interface.c Patched_LoadItemIconMain's ITEM_BIG_POE branch. The
//    icon segment layout is 0x1000 bytes per button, same as the N64 retail.
//
// 7. The ItemTableManager entry for GI_BIG_POE is overridden at init with
//    OBJECT_GI_BUTTERFLY + GID_BUTTERFLY, so the overhead item-get draw renders the
//    vanilla OoT butterfly mesh (slot 0x0175, which SoH already extracts). The itemId
//    stays ITEM_BIG_POE so Link still gets a bottle Beta Kokiri can trade for the Odd
//    Potion.
//
// Scope boundaries:
//
// - "SI_BUTTERFLY" as a dedicated shelf index (upstream Statics_PatchShop's end state)
//   still wants a SoH-source edit to z_en_girla.{c,h}; the module build has no patch
//   infrastructure for SoH source, so TickShopPatch rides SI_BIG_POE and leans on the
//   presentation layer to re-skin everywhere Link sees the item.
// - The Fast64 object_gi_butterfly patch at assets/model/object_gi_butterfly/ (the
//   texture-tile Gfx override from upstream) is not applied: we draw the vanilla
//   zeldaret butterfly straight. If the "patched to see the butterfly better" tweak
//   turns out needed in-game, follow-up with a GetItemEntry.drawFunc that calls the
//   patched DL before the vanilla one.
// - LOOKING_FOR_BUTTERFLY and SHOP_HAS_BUTTERFLY are module-static for now, same as
//   BETAKOKIRI_SPAWNED. The SaveManager custom section lands when the Beat 1 flag set is
//   worth plumbing together.

#include <array>
#include <cstring>

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/custom-message/CustomMessageManager.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/item-tables/ItemTableManager.h"
#include "soh/ShipInit.hpp"
#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "ButterflyIcon.inc.h"

extern "C" {
#include "TrifoolPercent.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "z64save.h"
#include "overlays/actors/ovl_En_Ossan/z_en_ossan.h"
#include "overlays/actors/ovl_En_GirlA/z_en_girla.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

// -----------------------------------------------------------------------------
// C-button icon swap.
// -----------------------------------------------------------------------------
// Mirrors upstream Triforce% scripts/statics/interface.c Patched_LoadItemIconMain:
// whenever a C button holds ITEM_BIG_POE, blit the butterfly texture over its slot in
// interfaceCtx->iconItemSegment so the HUD reads as the Butterfly bottle. The icon
// segment is laid out as 0x1000 bytes per button (32x32 RGBA32), same as the N64
// retail: see z64.h InterfaceContext.iconItemSegment. Idempotent -- the engine reloads
// the Big Poe icon when the equips change, and we re-stamp over it until the player
// equips something else.

// Number of equip-button slots SoH's SaveContext carries. z64save.h defines
// buttonItems[8] (vanilla 4 + 4 for D-pad equips).
static constexpr int kButtonSlotCount = 8;
// Only the C buttons feed the on-screen icon segment. Slots 0..2 are B / C-left /
// C-down (per the original Patched_LoadItemIconMain loop), slot 3 is C-right.
static constexpr int kIconSegmentButtonCount = 4;

static void SwapButterflyIconOnEquip() {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0 || gPlayState == nullptr) {
        return;
    }
    InterfaceContext* interfaceCtx = &gPlayState->interfaceCtx;
    if (interfaceCtx->iconItemSegment == nullptr) {
        return;
    }
    for (int button = 0; button < kIconSegmentButtonCount; ++button) {
        if (gSaveContext.equips.buttonItems[button] != ITEM_BIG_POE) {
            continue;
        }
        std::memcpy(interfaceCtx->iconItemSegment + button * 0x1000, kButterflyBottleIcon,
                    sizeof(kButterflyBottleIcon));
    }
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
// Walk the SCENE_BAZAAR (Hyrule Market Bazaar) NPC list each frame, find the shopkeeper
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
    if (gPlayState->sceneNum != SCENE_BAZAAR) {
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

    // Walk the NPC list for the shopkeeper, then force shelf 3 (vanilla SI_FAIRY) to
    // SI_BIG_POE so the player can buy a bottle whose contents chain into the Beta
    // Kokiri trade. The pending-sale item-get card goes through 0x5070 which the
    // custom-message hook renames to Butterfly, and ItemTable_Retrieve(GI_BIG_POE)
    // is overridden at init to draw object_gi_butterfly / GID_BUTTERFLY for the
    // "Link holds overhead" pose. We can't add a dedicated SI_BUTTERFLY enum from
    // outside the SoH source, so riding the Big Poe shop slot stays the module-only
    // approach -- the player sees "Butterfly" everywhere thanks to the dialogue +
    // icon + draw overrides.
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
    EnOssan* ossan = (EnOssan*)ossanActor;
    if (ossan->shelves == nullptr) {
        return; // shelves not loaded yet; retry next frame
    }
    EnGirlA* slot3 = ossan->shelfSlots[3];
    if (slot3 != nullptr && slot3->actor.params == SI_BIG_POE) {
        return; // already swapped
    }

    // Bazaar is store index 4; shelf-3 vanilla offset is (80, 76, -3). sItemShelfRot is
    // overlay-private, so reuse the rotation EnOssan already stamped on the existing
    // shelf slot (every retail re-spawn reads the same table, so this matches).
    s16 rotY = ossan->shelves->actor.shape.rot.y;
    s16 rotX = ossan->shelves->actor.shape.rot.x;
    s16 rotZ = ossan->shelves->actor.shape.rot.z;
    f32 spawnX = ossan->shelves->actor.world.pos.x + 80.0f;
    f32 spawnY = ossan->shelves->actor.world.pos.y + 76.0f;
    f32 spawnZ = ossan->shelves->actor.world.pos.z + -3.0f;
    if (slot3 != nullptr) {
        rotY = slot3->actor.shape.rot.y;
        rotX = slot3->actor.shape.rot.x;
        rotZ = slot3->actor.shape.rot.z;
        spawnX = slot3->actor.world.pos.x;
        spawnY = slot3->actor.world.pos.y;
        spawnZ = slot3->actor.world.pos.z;
        Actor_Kill(&slot3->actor);
    }
    ossan->shelfSlots[3] = (EnGirlA*)Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_GIRLA, spawnX,
                                                 spawnY, spawnZ, rotX, rotY, rotZ, SI_BIG_POE);
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
    CustomMessage msg(s, s, s, static_cast<TextBoxType>(type), static_cast<TextBoxPosition>(pos));
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
        // 0x5070 is the vanilla "You got a Big Poe!" item-get text id (EnGirlA's SI_BIG_POE
        // entry, textId pair 0x506F / 0x5070). Since the Butterfly rides the Big Poe slot
        // through the Bazaar sale, the item-get card actually fires on 0x5070 at purchase
        // time -- hook it here so the player sees the Butterfly flavor text. 0x0083 is kept
        // as a secondary route for any direct Message_StartTextbox caller (chests, dev
        // commands, future rando entries).
        case 0x5070:
        case 0x0083:
            LoadOneMessage(kButterflyGetText, sizeof(kButterflyGetText) - 1,
                           TEXTBOX_TYPE_BLUE, TEXTBOX_POS_BOTTOM, loadFromMessageTable);
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// GI_BIG_POE GetItemEntry override: swap the "Link holds overhead" draw to the
// vanilla OBJECT_GI_BUTTERFLY mesh (SoH object slot 0x0175, GID_BUTTERFLY 0x68).
// -----------------------------------------------------------------------------
// SoH's bottle-item draw dispatches on GetItemEntry.gid, so overriding the MOD_NONE
// entry for GI_BIG_POE flips the overhead model without touching player code. The
// actual item Link receives is still ITEM_BIG_POE (field 0x80 keeps the bottle-fill
// side-effect), so the Beta Kokiri trade pipeline that reads EXCH_ITEM_BIG_POE from
// the bottle slot keeps working. Gated on CVar via RegisterShipInitFunc's
// updatePaths: the init re-fires when the toggle flips, and this call becomes a
// no-op (and the entry is re-overridden) each time.
//
// Side-effect: a vanilla Big Poe pickup (ghost-shop sale, randomizer check) now
// also draws the butterfly mesh while the beat is active. Acceptable in the
// Triforce% context -- the save is dedicated to the TAS mode, and ITEM_BIG_POE is
// not shipped as a reward anywhere in the TrifoolPercent flow outside this trade.

static void RegisterButterflyItemEntry() {
    if (CVarGetInteger(CVAR_TRIFOOL_BUTTERFLY, 0) == 0) {
        return;
    }
    GetItemEntry butterflyEntry = GET_ITEM(ITEM_BIG_POE, OBJECT_GI_BUTTERFLY, GID_BUTTERFLY, 0x5070, 0x80,
                                           CHEST_ANIM_SHORT, ITEM_CATEGORY_MAJOR, MOD_NONE, GI_BIG_POE);
    ItemTableManager::Instance->AddItemEntry(MOD_NONE, GI_BIG_POE, butterflyEntry);
}

// -----------------------------------------------------------------------------
// Hooks + registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolBeat1_3() {
    RegisterButterflyItemEntry();
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(
        MoveOddPotionToChildIfNeeded);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickShopPatch);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(SwapButterflyIconOnEquip);
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

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterBeat1_3Widget() {
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Beat 1.3 - Butterfly trade", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_BUTTERFLY)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Turns on the Butterfly trade. Flow: Beta Kokiri's first greeting arms the "
            "hunt; the Bazaar shopkeeper's 'flying bug' text spawns a Big-Poe-slot bottle "
            "on shelf 3, re-skinned as the Butterfly end-to-end (C-button icon, overhead "
            "3D mesh from vanilla OBJECT_GI_BUTTERFLY, item-get card, shop and trade "
            "dialogue). The bottle Link carries chains into the Beta Kokiri trade -- "
            "offering it walks dialogue 0x0B13 and hands Link the Odd Potion, which is "
            "auto-moved into the child trade slot."));
}

