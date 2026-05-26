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
    engine::ui::Bindable<std::string> probeMagnetic;
    engine::ui::Bindable<std::string> probeMagneticDisplay{std::string("none")};
    engine::ui::Bindable<std::string> panelRight{std::string("12")};

    // Time.
    engine::ui::Bindable<std::string> pauseLabel;
    engine::ui::Bindable<std::string> timeScaleText;
    // Log-scale slider: the fraction maps to log2(scale) in [-3, 3], so x1 sits in the middle.
    engine::ui::Bindable<float> timeFrac;
    engine::ui::Bindable<std::string> timeFill;
    engine::ui::RelayCommand togglePause;
    engine::ui::RelayCommand step;

    // Scenes.
    engine::ui::RelayCommand presetDipole;
    engine::ui::RelayCommand presetLikePair;
    engine::ui::RelayCommand presetQuadrupole;
    engine::ui::RelayCommand presetCapacitor;
    engine::ui::RelayCommand presetOrbit;
    engine::ui::RelayCommand presetRutherford;
    engine::ui::RelayCommand presetSwarm;
    engine::ui::RelayCommand presetCyclotron;
    engine::ui::RelayCommand presetExBDrift;
    engine::ui::RelayCommand presetCoil;
    engine::ui::RelayCommand clearAll;
    engine::ui::RelayCommand openHelp;
    engine::ui::RelayCommand localeEn;
    engine::ui::RelayCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string("#6366f133")};
    engine::ui::Bindable<std::string> localeEnFg{std::string("#ffffff")};
    engine::ui::Bindable<std::string> localeUkBg{std::string("#ffffff14")};
    engine::ui::Bindable<std::string> localeUkFg{std::string("#aab1c3")};

    // Layer toggles (two-way: checkbox clicks write back).
    engine::ui::Bindable<bool> showPotential;
    engine::ui::Bindable<bool> showLines;
    engine::ui::Bindable<bool> showGrid;
    engine::ui::Bindable<bool> showFlow;
    engine::ui::Bindable<bool> showProbe;
    engine::ui::Bindable<bool> showTrails;
    engine::ui::Bindable<bool> showMagnetic;
    engine::ui::Bindable<bool> collisions;
    // Lorentz force. Off leaves the motion purely electric, whatever the Bz slider says.
    engine::ui::Bindable<bool> magnetic;

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
    // Signed slider: fraction 0.5 is Bz = 0. Motion uses it only while `magnetic` is on.
    engine::ui::Bindable<float> bFrac;
    engine::ui::Bindable<std::string> bText;
    engine::ui::Bindable<std::string> bFill;
    // Log-scale slider for the speed of light c; it sets μ₀/4π = k / c².
    engine::ui::Bindable<float> cFrac;
    engine::ui::Bindable<std::string> cText;
    engine::ui::Bindable<std::string> cFill;

    // Energy.
    engine::ui::Bindable<std::string> energyKinetic;
    engine::ui::Bindable<std::string> energyPotential;
    engine::ui::Bindable<std::string> energyTotal;
    engine::ui::Bindable<std::string> chargeCount;
    // Fastest free charge as a fraction of c: the quasi-static model holds only while this is small.
    engine::ui::Bindable<std::string> speedRatio;
};

}
