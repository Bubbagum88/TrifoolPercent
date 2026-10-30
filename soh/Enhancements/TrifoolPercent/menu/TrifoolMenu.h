#pragma once

// TrifoolPercent menu: a top-level SoH menu tab housing every TrifoolPercent toggle,
// cheat, troubleshooting tool and config knob. See docs/design/trifool-menu-structure.md
// for the full planned layout; this PR wires the frame + the sidebars that have real
// content today, and leaves the rest as "Coming soon" placeholders.

namespace TrifoolMenu {

// Each sidebar lives in its own TrifoolMenu<Section>.cpp and exposes one register fn.
// TrifoolMenu.cpp calls them in insertion order to build the tab.
void RegisterGeneral();
void RegisterScenes();
void RegisterBeats();
void RegisterCutscenes();
void RegisterMusic();
void RegisterQoL();
void RegisterCheats();
void RegisterInventory();
void RegisterDebug();

// Placeholder draw fn for sidebars that register their entry but have no content yet,
// so the nav row still renders and breakage of the AddSidebarEntry wiring is visible.
void DrawComingSoon();

// Per-feature widget registration fns, defined in the feature's own .cpp. Called from
// the matching Register<Sidebar>() above after AddSidebarEntry runs, so the sidebar is
// guaranteed to exist before AddWidget looks it up (fixes PR #101-era startup crash).
// Scenes & Rooms
void RegisterTestRoomSceneWidget();
void RegisterLostWoodsPathWidget();
// Beats & Progression
void RegisterBeat1_0Widget();
void RegisterBetaKokiriWidget();
void RegisterBeat1_3Widget();
void RegisterBeat2_0Widgets();
void RegisterBeat2_1Widget();
void RegisterMedallionFireWidget();
void RegisterMedallionForestWidget();
void RegisterMedallionLightWidget();
void RegisterMedallionWaterWidget();
void RegisterSongEngineWidget();
void RegisterBossRunningManWidgets();
// Cutscenes
void RegisterLongOfTimeCutsceneWidget();
void RegisterLongOfTimeAgeFlipWidget();
void RegisterTestRoomPrototypeCutsceneWidget();
void RegisterSacredRealmChainWidget();
void RegisterEndingWidget();
void RegisterStaffRollChainWidget();
// Dev & Debug
void RegisterSetupWidgets();

}  // namespace TrifoolMenu
