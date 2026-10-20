// TrifoolPercent / Music & Audio: reserved for the music engine. Stub for now.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterMusic() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Music & Audio", 1);
    WidgetPath path = { "TrifoolPercent", "Music & Audio", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "MusicComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
