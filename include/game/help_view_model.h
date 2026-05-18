#pragma once

#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>

namespace game {

inline constexpr std::size_t kHelpTopicCount = 13;

// One row of the help navigation list (assets/ui/help.xml, the nav ItemsControl).
class HelpNavItem final : public engine::ui::ViewModel {
public:
    HelpNavItem();

    engine::ui::Bindable<std::string> title;
    engine::ui::Bindable<std::string> background;
    engine::ui::Bindable<std::string> textColor;
    engine::ui::RelayCommand select;
};

// Data context of one topic file (assets/ui/help_topics/*.xml). Every topic is included through
// `<ItemTemplate src>`, so each one is an ItemsControl holding exactly one of these.
class HelpTopicViewModel final : public engine::ui::ViewModel {
public:
    HelpTopicViewModel();

    engine::ui::RelayCommand tryIt;
};

// Data context of assets/ui/help.xml. Member names must match the XML binding paths exactly.
class HelpViewModel final : public engine::ui::ViewModel {
public:
    HelpViewModel();

    engine::ui::Bindable<std::string> helpDisplay{std::string("none")};
    engine::ui::BindableList<std::shared_ptr<HelpNavItem>> navItems;
    engine::ui::RelayCommand closeHelp;
    engine::ui::Bindable<float> contentScroll;

    // One ItemsControl per topic file; shown with display:block-like values, hidden with "none".
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCharge;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCoulomb;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicField;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicSuperposition;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicFieldLines;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicPotential;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicEnergy;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicMotion;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicSoftening;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicCollisions;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicExperiments;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicUnits;
    engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>> topicLimits;

    engine::ui::Bindable<std::string> topicChargeDisplay;
    engine::ui::Bindable<std::string> topicCoulombDisplay;
    engine::ui::Bindable<std::string> topicFieldDisplay;
    engine::ui::Bindable<std::string> topicSuperpositionDisplay;
    engine::ui::Bindable<std::string> topicFieldLinesDisplay;
    engine::ui::Bindable<std::string> topicPotentialDisplay;
    engine::ui::Bindable<std::string> topicEnergyDisplay;
    engine::ui::Bindable<std::string> topicMotionDisplay;
    engine::ui::Bindable<std::string> topicSofteningDisplay;
    engine::ui::Bindable<std::string> topicCollisionsDisplay;
    engine::ui::Bindable<std::string> topicExperimentsDisplay;
    engine::ui::Bindable<std::string> topicUnitsDisplay;
    engine::ui::Bindable<std::string> topicLimitsDisplay;

    // The fields above in navigation order, so code can index them.
    std::array<engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>>*, kHelpTopicCount> topics;
    std::array<engine::ui::Bindable<std::string>*, kHelpTopicCount> topic_displays;
};

}
