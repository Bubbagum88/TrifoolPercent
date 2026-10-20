// TrifoolPercent / Scenes & Rooms: underlying scene enables + world patches.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "../TrifoolPercent.h"
#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterScenes() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Scenes & Rooms", 1);
    WidgetPath path = { "TrifoolPercent", "Scenes & Rooms", SECTION_COLUMN_1 };

    SohGui::mSohMenu->AddWidget(path, "Door of Time opens for adult Link", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_DOT_ADULT)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "On adult entry to Temple of Time, sets EVENTCHKINF_OPENED_DOOR_OF_TIME and "
            "roomCtx.drawParams[1] = 0xFF so the Door of Time reads open visually (open-door room "
            "mesh) and functionally (Door_Toki drops its collision on the next update). Fixes the "
            "vanilla oversight where Demo_Kankyo's cutscene gate only fires the opening animation "
            "for the child path, so a Setup / dev-warp as adult leaves the door closed."));
}
