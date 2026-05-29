#pragma once

#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>

namespace game {

inline constexpr std::size_t kHelpTopicCount = 23;
// "Predict, then check" questions per topic; answers stay collapsed until revealed.
inline constexpr std::size_t kHelpPredictionsPerTopic = 3;

// One row of the help navigation list: either a section header or a topic button. Both live in one
// ItemsControl; the unused half of the row is removed with display: none.
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

// Data context of one topic file (assets/ui/help_topics/*.xml). Every topic is included through
// `<ItemTemplate src>`, so each one is an ItemsControl holding exactly one of these.
class HelpTopicViewModel final : public engine::ui::ViewModel {
public:
    HelpTopicViewModel();

    engine::ui::RelayCommand tryIt;

    // Rewrites the show/hide labels from the active locale without changing which answers are open.
    void set_reveal_labels(std::string show, std::string hide);

    // Bindings reveal1..3 / answer1Display..3 / revealLabel1..3 for the "predict, then check" blocks.
    std::array<engine::ui::RelayCommand, kHelpPredictionsPerTopic> reveal;
    std::array<engine::ui::Bindable<std::string>, kHelpPredictionsPerTopic> answer_display;
    std::array<engine::ui::Bindable<std::string>, kHelpPredictionsPerTopic> reveal_label;

private:
    void toggle(std::size_t i);

    std::array<bool, kHelpPredictionsPerTopic> revealed_{};
    std::string show_;
    std::string hide_;
};

// Data context of assets/ui/help.xml. Member names must match the XML binding paths exactly.
class HelpViewModel final : public engine::ui::ViewModel {
public:
    HelpViewModel();

    engine::ui::Bindable<std::string> helpDisplay{std::string("none")};
    engine::ui::BindableList<std::shared_ptr<HelpNavItem>> navItems;
    engine::ui::RelayCommand closeHelp;
    engine::ui::RelayCommand localeEn;
    engine::ui::RelayCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string("#6366f133")};
    engine::ui::Bindable<std::string> localeEnFg{std::string("#ffffff")};
    engine::ui::Bindable<std::string> localeUkBg{std::string("#ffffff14")};
    engine::ui::Bindable<std::string> localeUkFg{std::string("#aab1c3")};
    // Exact px geometry of the help window (see Help::update_layout for why it is not CSS max-width).
    engine::ui::Bindable<std::string> windowLeft{std::string("36")};
    engine::ui::Bindable<std::string> windowWidth{std::string("1208")};
    engine::ui::Bindable<float> contentScroll;
    engine::ui::Bindable<float> navScroll;

    // One ItemsControl per topic file; shown with display:block-like values, hidden with "none".
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicUsing;
    // I. Basics
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCharge;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCoulomb;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicField;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicSuperposition;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicFieldLines;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGauss;
    // II. Potential and energy
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPotential;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPotentialEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicVoltage;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGradient;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPairEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicEnergyZero;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicWorkMotion;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicConfigEnergy;
    // III. Summary
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicGravity;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicOverview;
    // IV. Magnetism
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicBfield;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicLorentz;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCyclotron;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicExb;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCoil;
    // Appendix
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

    // The fields above in navigation order, so code can index them.
    std::array<engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>>*, kHelpTopicCount> topics;
    std::array<engine::ui::Bindable<std::string>*, kHelpTopicCount> topic_displays;
};

}
