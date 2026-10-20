// TrifoolPercent / Cutscenes: per-cutscene toggles + replay combo. Stub for now.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterCutscenes() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Cutscenes", 1);
    WidgetPath path = { "TrifoolPercent", "Cutscenes", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "CutscenesComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
