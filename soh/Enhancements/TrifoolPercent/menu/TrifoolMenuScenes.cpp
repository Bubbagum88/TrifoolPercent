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
    // Call per-feature widget fns first (they live in their feature's .cpp): this fn
    // owns the AddSidebarEntry above, so by the time these run the sidebar exists.
    TrifoolMenu::RegisterTestRoomSceneWidget();
    TrifoolMenu::RegisterLostWoodsPathWidget();

    WidgetPath path = { "TrifoolPercent", "Scenes & Rooms", SECTION_COLUMN_1 };

    SohGui::mSohMenu->AddWidget(path, "Door of Time opens for adult Link", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_DOT_ADULT)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "On adult entry to Temple of Time, sets EVENTCHKINF_OPENED_THE_DOOR_OF_TIME and "
            "roomCtx.unk_74[1] = 0xFF so the Door of Time reads open visually (open-door room "
            "mesh) and functionally (Door_Toki drops its collision on the next update). Fixes the "
            "vanilla oversight where Demo_Kankyo's cutscene gate only fires the opening animation "
            "for the child path, so a Setup / dev-warp as adult leaves the door closed."));
}
