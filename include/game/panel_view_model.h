#pragma once

#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <string>

namespace game {

// Data context of assets/ui/panel.xml. Member names must match the XML binding paths exactly:
// the generated assets::ui::Panel::bind() registers them by name. Game owns the simulation state
// and syncs it into (and, for toggles and sliders, back out of) these fields every frame.
class PanelViewModel final : public engine::ui::ViewModel {
public:
    PanelViewModel();

    // Status and cursor probe.
    engine::ui::Bindable<std::string> statusText;
    engine::ui::Bindable<std::string> statusColor;
    engine::ui::Bindable<std::string> sceneText;
    engine::ui::Bindable<std::string> probeLeft;
    engine::ui::Bindable<std::string> probeTop;
    engine::ui::Bindable<std::string> probeVisibility{std::string("hidden")};
    engine::ui::Bindable<std::string> probeField;
    engine::ui::Bindable<std::string> probeComponents;
    engine::ui::Bindable<std::string> probePotential;
    engine::ui::Bindable<std::string> panelRight{std::string("12")};

    // Time.
    engine::ui::Bindable<std::string> pauseLabel;
    engine::ui::Bindable<std::string> timeScaleText;
    engine::ui::RelayCommand togglePause;
    engine::ui::RelayCommand step;
    engine::ui::RelayCommand slower;
    engine::ui::RelayCommand faster;

    // Scenes.
    engine::ui::RelayCommand presetDipole;
    engine::ui::RelayCommand presetLikePair;
    engine::ui::RelayCommand presetQuadrupole;
    engine::ui::RelayCommand presetCapacitor;
    engine::ui::RelayCommand presetOrbit;
    engine::ui::RelayCommand presetRutherford;
    engine::ui::RelayCommand presetSwarm;
    engine::ui::RelayCommand clearAll;
    engine::ui::RelayCommand openHelp;

    // Layer toggles (two-way: checkbox clicks write back).
    engine::ui::Bindable<bool> showPotential;
    engine::ui::Bindable<bool> showLines;
    engine::ui::Bindable<bool> showGrid;
    engine::ui::Bindable<bool> showFlow;
    engine::ui::Bindable<bool> showProbe;
    engine::ui::Bindable<bool> showTrails;
    engine::ui::Bindable<bool> collisions;

    // Sliders: *Frac is written by the drag binding in [0, 1]; *Fill is the CSS width of the bar.
    engine::ui::Bindable<float> kFrac;
    engine::ui::Bindable<std::string> kText;
    engine::ui::Bindable<std::string> kFill;
    engine::ui::Bindable<float> epsFrac;
    engine::ui::Bindable<std::string> epsText;
    engine::ui::Bindable<std::string> epsFill;
    engine::ui::Bindable<float> newChargeFrac;
    engine::ui::Bindable<std::string> newChargeText;
    engine::ui::Bindable<std::string> newChargeFill;

    // Energy.
    engine::ui::Bindable<std::string> energyKinetic;
    engine::ui::Bindable<std::string> energyPotential;
    engine::ui::Bindable<std::string> energyTotal;
    engine::ui::Bindable<std::string> chargeCount;
};

}
