// TrifoolPercent / Cheats: grant Triforce, grant medallions, warp to beat. Stub for now.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterCheats() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Cheats", 1);
    WidgetPath path = { "TrifoolPercent", "Cheats", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "CheatsComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
