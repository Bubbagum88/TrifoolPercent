// TrifoolPercent Setup slice: the quest's starting state, the pause-screen debug inventory,
// and the dev-input shortcuts (Arwing + jump-to-beat warps).
//
// Adapted for Ship of Harkinian from Triforce% (statics/statics.c), (C) 2019-2022
// The Triforce% Team, used under the terms of upstream/TRIFORCE-PERCENT-LICENSE.txt.
// The upstream functions this file adapts: Statics_SetGameState, Statics_InventoryEditor,
// Statics_TestShortcuts.

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
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

// Convenience predicates for the CVars this slice owns.
static bool PresetEnabled() { return CVarGetInteger(CVAR_TRIFOOL_SETUP_PRESET, 0) != 0; }
static bool DevInputsEnabled() { return CVarGetInteger(CVAR_TRIFOOL_DEV_INPUTS, 0) != 0; }
static bool PauseDebugInvEnabled() { return CVarGetInteger(CVAR_TRIFOOL_PAUSE_DEBUG_INV, 0) != 0; }

// Statics_SetGameState adaptation. The upstream run started adult Link mid-quest with every
// pre-Triforce side quest resolved: Epona won, DMT fire out, carpenters rescued, masks sold,
// magic + double magic available, and B held the Biggoron Sword so Link never carried the
// Master Sword. This is only applied once per save, and only when the preset CVar is on.
static void ApplyTrifoolSetupPreset() {
    static u32 sApplied = 0;
    if (sApplied == gSaveContext.fileNum + 1) {
        return;
    }
    if (!PresetEnabled()) {
        return;
    }
    sApplied = gSaveContext.fileNum + 1;

    // Progress flags needed so the showcase beats are reachable without replaying OoT.
    gSaveContext.eventChkInf[0x1] |= 0x0100;              // Ingo / Epona
    gSaveContext.eventChkInf[0x2] |= 0x8000;              // Death Mountain not on fire
    gSaveContext.eventChkInf[0x9] |= 0x000F;              // Carpenters rescued
    gSaveContext.eventChkInf[0xA] |= 0x0011 | (1 << 8) | (1 << 5);
    gSaveContext.eventChkInf[0xB] |= 0x010C | (1 << 0xB) | (1 << 0xC);
    gSaveContext.eventChkInf[0xC] |= 0x0002;              // Saria on Lost Woods Bridge
    gSaveContext.itemGetInf[0x1] |= 0x0008;               // Deku Seeds picked up
    gSaveContext.itemGetInf[0x2] |= 0x0478;               // All trading masks obtained
    gSaveContext.itemGetInf[0x3] |= 0x8F00;               // All masks sold
    gSaveContext.infTable[25]     |= 0x0100;              // Magic jar picked up

    // Adult inventory: B = Biggoron Sword (no Master Sword), bombs + ocarina on C.
    gSaveContext.adultEquips.buttonItems[0] = ITEM_SWORD_BGS;
    gSaveContext.adultEquips.buttonItems[1] = ITEM_NONE;
    gSaveContext.adultEquips.buttonItems[2] = ITEM_BOMB;
    gSaveContext.adultEquips.buttonItems[3] = ITEM_OCARINA_TIME;
    gSaveContext.adultEquips.cButtonSlots[0] = SLOT_NONE;
    gSaveContext.adultEquips.cButtonSlots[1] = SLOT_BOMB;
    gSaveContext.adultEquips.cButtonSlots[2] = SLOT_OCARINA_TIME;
    gSaveContext.adultEquips.equipment = 0x1103;
    gSaveContext.bgsFlag = 1;
    gSaveContext.swordHealth = 8;

    // Magic + double magic.
    gSaveContext.magicAcquired = 1;
    gSaveContext.magicLevel = 2;
    gSaveContext.magic = 0x60;
    gSaveContext.doubleMagic = 1;

    // Keep Navi quiet.
    gSaveContext.naviTimer = 1;

    // Hand over the Sages' Charm when the Beat 2.0 slice is on, so testers can exercise
    // the medallion-on-C / Din's-Fire-on-C flow (and any medallion-ability-gating that
    // keys on it) without first beating the Running Man boss. Session-scoped, same as
    // sHasSagesCharm itself; the Beat 1 SaveManager custom-section pass will later make
    // it durable.
    if (CVarGetInteger(CVAR_TRIFOOL_SAGES_CHARM, 0) != 0) {
        TrifoolBeat_GrantSagesCharm();
    }
}

// Reads the Player 1 controller's current frame. Safe to call from a per-frame hook.
static Input* PlayerInput() {
    if (gPlayState == nullptr) {
        return nullptr;
    }
    return &gPlayState->state.input[0];
}

static bool Held(u16 button) {
    Input* in = PlayerInput();
    return in != nullptr && (in->cur.button & button);
}

static bool Pressed(u16 button) {
    Input* in = PlayerInput();
    return in != nullptr && (in->press.button & button);
}

// Statics_InventoryEditor adaptation. Pressing L while the pause screen is up flips OoT's
// vanilla, unused debug-inventory editor on by setting pauseCtx.debugState = 1. The upstream
// did its own L-edge tracking because it was polling on the VI; GameInteractor's per-frame
// hook is already game-synchronous so Pressed(BTN_L) is enough.
static void TickPauseDebugInventory() {
    if (!PauseDebugInvEnabled() || gPlayState == nullptr) {
        return;
    }
    PauseContext* pause = &gPlayState->pauseCtx;
    if (pause->state == 0 || pause->debugState != 0) {
        return;
    }
    if (Pressed(BTN_L)) {
        pause->debugState = 1;
    }
}

// Jumps to one of the showcase beats. Each entrance mirrors the upstream Statics_TestShortcuts
// commented-out block; cutsceneIndex 0xFFF0 forces the Chamber of Sages cutscene when applicable.
struct DevWarp {
    const char* label;
    s32 entranceIndex;
    s32 linkAgeOnLoad; // 0 = adult, 1 = child
    s16 cutsceneIndex; // 0 = none
};

static const DevWarp kDevWarps[] = {
    { "Lost Woods (child)",      ENTR_LOST_WOODS_0,           1,  0      },
    { "Hyrule Market Bazaar",    ENTR_BAZAAR_0,               1,  0      },
    { "Thieves' Hideout (Nabooru)", 0x04A6,                   1,  0      },
    { "Zora's Domain (adult)",   ENTR_ZORAS_DOMAIN_0,         0,  0      },
    { "Unicorn Fountain",        0x0560,                      0,  0      },
    { "Chamber of Sages",        0x006B,                      0,  (s16)0xFFF0 },
    { "Staff Roll",              0x00CD,                      1,  (s16)0xFFF8 },
    { "Test Room (hex hub)",     ENTR_SYOTES_0,               1,  0      },
};

static constexpr size_t kDevWarpCount = sizeof(kDevWarps) / sizeof(kDevWarps[0]);

static void TriggerDevWarp(size_t index) {
    if (gPlayState == nullptr || index >= kDevWarpCount) {
        return;
    }
    const DevWarp& warp = kDevWarps[index];
    gPlayState->linkAgeOnLoad = warp.linkAgeOnLoad;
    gPlayState->nextEntranceIndex = warp.entranceIndex;
    if (warp.cutsceneIndex != 0) {
        gSaveContext.cutsceneIndex = (u16)warp.cutsceneIndex;
    }
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK;
}

// Statics_TestShortcuts adaptation. All chords are gated by L (and no R/Z, so the camera and
// targeting keep working untouched):
//   L + D-Down            spawn the Arwing (vanilla En_Clear_Tag)
//   L + D-Right           cycle to the next dev-warp slot
//   L + D-Right + A       jump to the currently selected dev-warp slot
//   L + D-Left            print the current dev-warp slot to the on-screen HUD (future)
static void TickDevInputs() {
    if (!DevInputsEnabled() || gPlayState == nullptr) {
        return;
    }
    if (!Held(BTN_L) || Held(BTN_R) || Held(BTN_Z)) {
        return;
    }

    if (Pressed(BTN_DDOWN)) {
        Actor* existing = Actor_Find(&gPlayState->actorCtx, ACTOR_EN_CLEAR_TAG, ACTORCAT_BOSS);
        if (existing == nullptr) {
            Player* player = GET_PLAYER(gPlayState);
            Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_CLEAR_TAG,
                        player->actor.world.pos.x, player->actor.world.pos.y + 120.0f,
                        player->actor.world.pos.z, 0, 0, 0, 0);
        }
    }

    static size_t sDevWarpSlot = 0;
    if (Pressed(BTN_DRIGHT)) {
        if (Held(BTN_A)) {
            TriggerDevWarp(sDevWarpSlot);
        } else {
            sDevWarpSlot = (sDevWarpSlot + 1) % kDevWarpCount;
        }
    }
}

static void RegisterTrifoolSetup() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { ApplyTrifoolSetupPreset(); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickPauseDebugInventory);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(TickDevInputs);
}

static RegisterShipInitFunc setupInitFunc(RegisterTrifoolSetup,
                                          { CVAR_TRIFOOL_SETUP_PRESET, CVAR_TRIFOOL_DEV_INPUTS,
                                            CVAR_TRIFOOL_PAUSE_DEBUG_INV });

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

static void RegisterTrifoolSetupWidgets() {
    WidgetPath path = { "Enhancements", "TrifoolPercent", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Setup Preset (new save)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_SETUP_PRESET)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "On the first OnLoadGame of a save with this on, write the Triforce% showcase "
            "starting state: Epona, DMT fire out, carpenters, masks sold, magic + double magic, "
            "adult B = Biggoron Sword (no Master Sword), Navi silent."));
    SohGui::mSohMenu->AddWidget(path, "Pause-screen L opens debug inventory", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_PAUSE_DEBUG_INV)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Enables OoT's unused in-game inventory editor on the pause screen (L to open), "
            "the \"dev screen\" from the Triforce% showcase."));
    SohGui::mSohMenu->AddWidget(path, "Dev inputs (Arwing + warps)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_DEV_INPUTS)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "L+D-Down spawns the Arwing (vanilla En_Clear_Tag). L+D-Right cycles through the "
            "showcase beat warps (Lost Woods, Bazaar, Thieves' Hideout, Zora's Domain, Unicorn "
            "Fountain, Chamber of Sages, Staff Roll); L+D-Right+A jumps to the current slot."));
}

static RegisterMenuInitFunc setupMenuInitFunc(RegisterTrifoolSetupWidgets);
