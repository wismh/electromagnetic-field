#pragma once

#include <engine/ui/bindable.h>
#include <engine/ui/view_model.h>

#include <game/locale_style.h>
#include <game/panel_command.h>

#include <string>

namespace game {

// Member names must match the binding paths in assets/ui/panel.xml.
class PanelViewModel final : public engine::ui::ViewModel {
public:
    PanelViewModel();

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

    engine::ui::Bindable<std::string> pauseLabel;
    engine::ui::Bindable<std::string> timeScaleText;
    // log2(scale) in [-3, 3]; x1 is the middle.
    engine::ui::Bindable<float> timeFrac;
    engine::ui::Bindable<std::string> timeFill;
    PanelCommand togglePause;
    PanelCommand step;

    PanelCommand clearAll;
    PanelCommand openScenes;
    PanelCommand openHelp;
    PanelCommand localeEn;
    PanelCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string(kLocaleOnBg)};
    engine::ui::Bindable<std::string> localeEnFg{std::string(kLocaleOnFg)};
    engine::ui::Bindable<std::string> localeUkBg{std::string(kLocaleOffBg)};
    engine::ui::Bindable<std::string> localeUkFg{std::string(kLocaleOffFg)};

    engine::ui::Bindable<bool> showPotential;
    engine::ui::Bindable<bool> showLines;
    engine::ui::Bindable<bool> showGrid;
    engine::ui::Bindable<bool> showFlow;
    engine::ui::Bindable<bool> showProbe;
    engine::ui::Bindable<bool> showTrails;
    engine::ui::Bindable<bool> showMagnetic;
    engine::ui::Bindable<bool> collisions;
    // Off: motion stays electric, whatever the Bz slider says.
    engine::ui::Bindable<bool> magnetic;

    // *Frac is the drag value in [0, 1]; *Fill is the bar width.
    engine::ui::Bindable<float> kFrac;
    engine::ui::Bindable<std::string> kText;
    engine::ui::Bindable<std::string> kFill;
    engine::ui::Bindable<float> epsFrac;
    engine::ui::Bindable<std::string> epsText;
    engine::ui::Bindable<std::string> epsFill;
    engine::ui::Bindable<float> newChargeFrac;
    engine::ui::Bindable<std::string> newChargeText;
    engine::ui::Bindable<std::string> newChargeFill;
    // Fraction 0.5 is Bz = 0. Used only while magnetic is on.
    engine::ui::Bindable<float> bFrac;
    engine::ui::Bindable<std::string> bText;
    engine::ui::Bindable<std::string> bFill;
    // log10(c); sets μ₀/4π = k / c².
    engine::ui::Bindable<float> cFrac;
    engine::ui::Bindable<std::string> cText;
    engine::ui::Bindable<std::string> cFill;

    engine::ui::Bindable<std::string> energyKinetic;
    engine::ui::Bindable<std::string> energyPotential;
    engine::ui::Bindable<std::string> energyTotal;
    engine::ui::Bindable<std::string> chargeCount;
    // Fastest free charge as v/c.
    engine::ui::Bindable<std::string> speedRatio;
};

}
