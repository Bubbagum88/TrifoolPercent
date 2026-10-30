// TrifoolPercent / Cutscenes: per-cutscene toggles. Widget bodies live in each cutscene's .cpp.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterCutscenes() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Cutscenes", 1);
    TrifoolMenu::RegisterLongOfTimeCutsceneWidget();
    TrifoolMenu::RegisterLongOfTimeAgeFlipWidget();
    TrifoolMenu::RegisterTestRoomPrototypeCutsceneWidget();
    TrifoolMenu::RegisterSacredRealmChainWidget();
    TrifoolMenu::RegisterEndingWidget();
    TrifoolMenu::RegisterStaffRollChainWidget();
}
