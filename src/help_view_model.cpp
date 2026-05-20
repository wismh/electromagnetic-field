#include <game/help_view_model.h>

#include <engine/ui/binding_id.h>

#include <asset_ids.h>

#include <string>

namespace game {
namespace {

constexpr const char* kShowAnswer = "Показати відповідь";
constexpr const char* kHideAnswer = "Сховати відповідь";

}

// Item view models are registered by hand: their bindings live in included template files, and
// one class serves every nav row / every topic file.
HelpNavItem::HelpNavItem() {
    property(engine::ui::intern("title"), title);
    property(engine::ui::intern("background"), background);
    property(engine::ui::intern("textColor"), textColor);
    property(engine::ui::intern("headerDisplay"), headerDisplay);
    property(engine::ui::intern("topicDisplay"), topicDisplay);
    command(engine::ui::intern("select"), select);
}

HelpTopicViewModel::HelpTopicViewModel() {
    command(engine::ui::intern("tryIt"), tryIt);
    for (std::size_t i = 0; i < kHelpPredictionsPerTopic; ++i) {
        const std::string n = std::to_string(i + 1);
        answer_display[i] = std::string("none");
        reveal_label[i] = std::string(kShowAnswer);
        reveal[i] = [this, i] { toggle(i); };
        command(engine::ui::intern("reveal" + n), reveal[i]);
        property(engine::ui::intern("answer" + n + "Display"), answer_display[i]);
        property(engine::ui::intern("revealLabel" + n), reveal_label[i]);
    }
}

void HelpTopicViewModel::toggle(std::size_t i) {
    revealed_[i] = !revealed_[i];
    answer_display[i] = std::string(revealed_[i] ? "block" : "none");
    reveal_label[i] = std::string(revealed_[i] ? kHideAnswer : kShowAnswer);
}

HelpViewModel::HelpViewModel() :
    topics{
            &topicCharge,
            &topicCoulomb,
            &topicField,
            &topicSuperposition,
            &topicFieldLines,
            &topicGauss,
            &topicPotential,
            &topicPotentialEnergy,
            &topicVoltage,
            &topicGradient,
            &topicPairEnergy,
            &topicEnergyZero,
            &topicWorkMotion,
            &topicConfigEnergy,
            &topicGravity,
            &topicOverview,
            &topicSimulation,
    },
    topic_displays{
            &topicChargeDisplay,
            &topicCoulombDisplay,
            &topicFieldDisplay,
            &topicSuperpositionDisplay,
            &topicFieldLinesDisplay,
            &topicGaussDisplay,
            &topicPotentialDisplay,
            &topicPotentialEnergyDisplay,
            &topicVoltageDisplay,
            &topicGradientDisplay,
            &topicPairEnergyDisplay,
            &topicEnergyZeroDisplay,
            &topicWorkMotionDisplay,
            &topicConfigEnergyDisplay,
            &topicGravityDisplay,
            &topicOverviewDisplay,
            &topicSimulationDisplay,
    } {
    assets::ui::Help::bind(*this);
}

}
