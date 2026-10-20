// TrifoolPercent / Quality of Life: mod-specific QoL. Stub for now.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterQoL() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Quality of Life", 1);
    WidgetPath path = { "TrifoolPercent", "Quality of Life", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "QoLComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
