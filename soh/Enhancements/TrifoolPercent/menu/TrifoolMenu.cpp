// Top-level TrifoolPercent menu tab. Registers the tab entry, invokes each sidebar's
// register fn in insertion order, and installs the whole thing via RegisterMenuInitFunc.

#include <imgui.h>

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

namespace TrifoolMenu {

void DrawComingSoon() {
    ImGui::TextDisabled("Coming soon");
}

}  // namespace TrifoolMenu

static void RegisterTrifoolMenu() {
    SohGui::mSohMenu->AddMenuEntry("TrifoolPercent", CVAR_SETTING("Menu.TrifoolPercentSidebarSection"));
    TrifoolMenu::RegisterGeneral();
    TrifoolMenu::RegisterScenes();
    TrifoolMenu::RegisterBeats();
    TrifoolMenu::RegisterCutscenes();
    TrifoolMenu::RegisterMusic();
    TrifoolMenu::RegisterQoL();
    TrifoolMenu::RegisterCheats();
    TrifoolMenu::RegisterInventory();
    TrifoolMenu::RegisterDebug();
}

static RegisterMenuInitFunc trifoolMenuInitFunc(RegisterTrifoolMenu);
