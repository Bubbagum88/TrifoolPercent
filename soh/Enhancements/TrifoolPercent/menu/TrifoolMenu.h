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

}  // namespace TrifoolMenu
