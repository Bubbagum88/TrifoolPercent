// TrifoolPercent / Beats & Progression: beats, songs, medallions, Sages' Charm.
// Stub for now; widgets land as each beat's CVars are audited for safe default surfacing.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterBeats() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Beats & Progression", 1);
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "BeatsComingSoon", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo&) { TrifoolMenu::DrawComingSoon(); });
}
