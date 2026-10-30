// TrifoolPercent / Dev & Debug: dev-only widgets, test actors, dev-warps.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "../TrifoolPercent.h"
#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterDebug() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Dev & Debug", 1);
    TrifoolMenu::RegisterSetupWidgets();

    WidgetPath path = { "TrifoolPercent", "Dev & Debug", SECTION_COLUMN_1 };

    SohGui::mSohMenu->AddWidget(path, "Mimic Chest in Test Room", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_MIMIC_CHEST)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Spawns the Triforce% MimicChest at the centre of the hex Test Room (SCENE_SYOTES). It "
            "idles until Link steps inside ~100 units, then switches to the attack animation and "
            "lurches toward him until he backs off ~300 units. Object data is compiled directly into "
            "the SoH module -- no new .o2r is required."));
}
