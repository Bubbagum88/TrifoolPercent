// TrifoolPercent / Beats & Progression: beats, songs, medallions, Sages' Charm.
// Widget bodies live in each feature's own .cpp (Beat1_0Onboarding.cpp, MedallionFire.cpp, ...).
// This sidebar's register fn adds the sidebar entry, then calls each feature's register fn in the
// order the design doc lists them. Running them from here guarantees the sidebar exists before
// AddWidget looks it up (fixes the startup .at() crash from stale "Enhancements" paths).

#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include "TrifoolMenu.h"

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

void TrifoolMenu::RegisterBeats() {
    SohGui::mSohMenu->AddSidebarEntry("TrifoolPercent", "Beats & Progression", 1);
    TrifoolMenu::RegisterBeat1_0Widget();
    TrifoolMenu::RegisterBetaKokiriWidget();
    TrifoolMenu::RegisterBeat1_3Widget();
    TrifoolMenu::RegisterBeat2_0Widgets();
    TrifoolMenu::RegisterBeat2_1Widget();
    TrifoolMenu::RegisterMedallionFireWidget();
    TrifoolMenu::RegisterMedallionForestWidget();
    TrifoolMenu::RegisterMedallionLightWidget();
    TrifoolMenu::RegisterMedallionWaterWidget();
    TrifoolMenu::RegisterSongEngineWidget();
    TrifoolMenu::RegisterBossRunningManWidgets();
}
