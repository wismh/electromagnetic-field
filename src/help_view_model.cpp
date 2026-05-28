#include <game/help_view_model.h>

#include <engine/ui/binding_id.h>

#include <asset_ids.h>

#include <string>

namespace game {

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
        reveal[i] = [this, i] { toggle(i); };
        command(engine::ui::intern("reveal" + n), reveal[i]);
        property(engine::ui::intern("answer" + n + "Display"), answer_display[i]);
        property(engine::ui::intern("revealLabel" + n), reveal_label[i]);
    }
}

void HelpTopicViewModel::set_reveal_labels(std::string show, std::string hide) {
    show_ = std::move(show);
    hide_ = std::move(hide);
    for (std::size_t i = 0; i < kHelpPredictionsPerTopic; ++i) {
        reveal_label[i] = revealed_[i] ? hide_ : show_;
    }
}

void HelpTopicViewModel::toggle(std::size_t i) {
    revealed_[i] = !revealed_[i];
    answer_display[i] = std::string(revealed_[i] ? "block" : "none");
    reveal_label[i] = revealed_[i] ? hide_ : show_;
}

HelpViewModel::HelpViewModel() :
    topics{
            &topicUsing,
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
            &topicUsingDisplay,
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
    // The asset codegen does not collect scroll-x / scroll-y bindings into bind(), so these two are
    // registered by hand (docs/engine-limits.md).
    property(engine::ui::intern("contentScroll"), contentScroll);
    property(engine::ui::intern("navScroll"), navScroll);
}

}
