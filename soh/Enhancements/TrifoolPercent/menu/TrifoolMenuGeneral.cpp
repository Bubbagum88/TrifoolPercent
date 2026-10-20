// TrifoolPercent / General: the experience toggles.

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "../TrifoolPercent.h"
#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterGeneral() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "General", 1);
    WidgetPath path = { "TrifoolPercent", "General", SECTION_COLUMN_1 };

    SohGui::mSohMenu->AddWidget(path, "Triforce Room", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TRIFORCE_ROOM)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "The Triforce% Triforce Room (in the debug Test Room's slot): the Triforce pieces, chest and light, the "
            "\"You got the Triforce!!!\" cutscene and the goddesses' wish. Needs TrifoolPercent.o2r in the mods "
            "folder for the room itself."));

    SohGui::mSohMenu->AddWidget(path, "Test Flame by Link's House", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_TEST_ENTRANCE)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "Spawns a flame in front of Link's house in Kokiri Forest, drawn from vanilla OoT's unused "
            "object_fire slot (TCRF \"Dancing Flame\"). Walking into it warps Link into SCENE_SYOTES "
            "(the debug Test Room slot) where the hex hub exits to every WIP TrifoolPercent room. "
            "Replaces the two POC-era flames that pointed at the Triforce Room and Unicorn Fountain directly."));

    SohGui::mSohMenu->AddWidget(path, "Unicorn Fountain", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_UNICORN_FOUNTAIN)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "The Triforce% Unicorn Fountain (in the debug Test Map 1 slot): the Beta Great Fairy who teaches the "
            "Overture of Sages. Needs TrifoolPercent.o2r in the mods folder for the room itself. The fairy's mesh "
            "and the cutscene are not available yet; a column of golden sparkles stands in for her."));

    SohGui::mSohMenu->AddWidget(path, "Unicorn Fountain entry cutscene", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_FOUNTAIN_ENTRY_CS)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(true).Tooltip(
            "Plays a 240-frame camera-only arrival pan when Link enters the Unicorn Fountain. The fairy "
            "holds off rising until the cutscene finishes. Turn off to iterate on the room without the "
            "cutscene running every entry. A Fast64 export of UnicornFountain.blend will replace the "
            "hand-written data in a later pass; the trigger stays the same."));

    SohGui::mSohMenu->AddWidget(path, "Overworld entrance to Unicorn Fountain", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_FOUNTAIN_OVERWORLD)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Spawns a TfUnusedFlame at the bottom of Zora's Domain -- the fan-rumored secret-entrance "
            "spot on the basin wall -- that warps Link into the Unicorn Fountain scene. Mirrors the "
            "upstream Triforce% route (which patched the Zora's Fountain entrance 0x0560 so leaving "
            "Zora's Domain after the Fire Medallion thaw landed Link in the fountain) while leaving "
            "the vanilla entrance table untouched. Coordinates are provisional -- tune in-game if the "
            "flame lands behind a wall. A dev-convenience flame also lives in the hex Test Room."));
}
