#pragma once

#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <game/locale_style.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>

namespace game {

inline constexpr std::size_t kHelpTopicCount = 23;
inline constexpr std::size_t kHelpPredictionsPerTopic = 3;

// Header or topic button; the unused half is display: none.
class HelpNavItem final : public engine::ui::ViewModel {
public:
    HelpNavItem();

    engine::ui::Bindable<std::string> title;
    engine::ui::Bindable<std::string> background;
    engine::ui::Bindable<std::string> textColor;
    engine::ui::Bindable<std::string> headerDisplay{std::string("none")};
    engine::ui::Bindable<std::string> topicDisplay{std::string("block")};
    engine::ui::RelayCommand select;
};

class HelpTopicViewModel final : public engine::ui::ViewModel {
public:
    HelpTopicViewModel();

    engine::ui::RelayCommand tryIt;

    void set_reveal_labels(std::string show, std::string hide);

    // Answer stays at height 0 until revealed.
    std::array<engine::ui::RelayCommand, kHelpPredictionsPerTopic> reveal;
    std::array<engine::ui::Bindable<std::string>, kHelpPredictionsPerTopic> answer_height;
    std::array<engine::ui::Bindable<std::string>, kHelpPredictionsPerTopic> answer_gap;
    std::array<engine::ui::Bindable<std::string>, kHelpPredictionsPerTopic> reveal_label;

    void note_answer_height(std::size_t index, float height);
    void apply_answer_motion();

private:
    void toggle(std::size_t i);

    std::array<bool, kHelpPredictionsPerTopic> revealed_{};
    std::array<float, kHelpPredictionsPerTopic> natural_{};
    std::string show_;
    std::string hide_;
};

// Member names must match the binding paths in assets/ui/help.xml.
class HelpViewModel final : public engine::ui::ViewModel {
public:
    HelpViewModel();

    engine::ui::Bindable<std::string> helpDisplay{std::string("none")};
    engine::ui::Bindable<std::string> windowPop{std::string("scale(0)")};
    engine::ui::Bindable<std::string> backdropDim{std::string("0")};
    engine::ui::BindableList<std::shared_ptr<HelpNavItem>> navItems;
    engine::ui::RelayCommand closeHelp;
    engine::ui::RelayCommand localeEn;
    engine::ui::RelayCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string(kLocaleOnBg)};
    engine::ui::Bindable<std::string> localeEnFg{std::string(kLocaleOnFg)};
    engine::ui::Bindable<std::string> localeUkBg{std::string(kLocaleOffBg)};
    engine::ui::Bindable<std::string> localeUkFg{std::string(kLocaleOffFg)};
    engine::ui::Bindable<std::string> windowLeft{std::string("36")};
    engine::ui::Bindable<std::string> windowWidth{std::string("1208")};
    engine::ui::Bindable<float> contentScroll;
    engine::ui::Bindable<float> navScroll;

    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicUsing;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCharge;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCoulomb;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicField;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicSuperposition;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicFieldLines;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGauss;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPotential;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPotentialEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicVoltage;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGradient;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPairEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicEnergyZero;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicWorkMotion;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicConfigEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicBfield;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicLorentz;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCyclotron;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicExb;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCoil;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGravity;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicOverview;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicSimulation;

    engine::ui::Bindable<std::string> topicUsingDisplay;
    engine::ui::Bindable<std::string> topicChargeDisplay;
    engine::ui::Bindable<std::string> topicCoulombDisplay;
    engine::ui::Bindable<std::string> topicFieldDisplay;
    engine::ui::Bindable<std::string> topicSuperpositionDisplay;
    engine::ui::Bindable<std::string> topicFieldLinesDisplay;
    engine::ui::Bindable<std::string> topicGaussDisplay;
    engine::ui::Bindable<std::string> topicPotentialDisplay;
    engine::ui::Bindable<std::string> topicPotentialEnergyDisplay;
    engine::ui::Bindable<std::string> topicVoltageDisplay;
    engine::ui::Bindable<std::string> topicGradientDisplay;
    engine::ui::Bindable<std::string> topicPairEnergyDisplay;
    engine::ui::Bindable<std::string> topicEnergyZeroDisplay;
    engine::ui::Bindable<std::string> topicWorkMotionDisplay;
    engine::ui::Bindable<std::string> topicConfigEnergyDisplay;
    engine::ui::Bindable<std::string> topicGravityDisplay;
    engine::ui::Bindable<std::string> topicOverviewDisplay;
    engine::ui::Bindable<std::string> topicBfieldDisplay;
    engine::ui::Bindable<std::string> topicLorentzDisplay;
    engine::ui::Bindable<std::string> topicCyclotronDisplay;
    engine::ui::Bindable<std::string> topicExbDisplay;
    engine::ui::Bindable<std::string> topicCoilDisplay;
    engine::ui::Bindable<std::string> topicSimulationDisplay;

    // topics / topic_displays, in navigation order.
    std::array<engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>>*, kHelpTopicCount> topics;
    std::array<engine::ui::Bindable<std::string>*, kHelpTopicCount> topic_displays;
};

}
