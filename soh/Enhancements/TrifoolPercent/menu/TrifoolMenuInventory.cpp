// TrifoolPercent / Inventory & Flags: popout window for TrifoolPercent state. Stub for now.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterInventory() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Inventory & Flags", 1);
    WidgetPath path = { "TrifoolPercent", "Inventory & Flags", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "InventoryComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
