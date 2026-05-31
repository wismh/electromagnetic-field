#include <game/help_view_model.h>

#include <engine/ui/binding_id.h>

#include <asset_ids.h>

#include <cmath>
#include <format>
#include <string>

namespace game {

// Included templates are not in the generated bind().
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
        answer_height[i] = std::string("0");
        answer_gap[i] = std::string("0");
        reveal[i] = [this, i] { toggle(i); };
        command(engine::ui::intern("reveal" + n), reveal[i]);
        property(engine::ui::intern("answer" + n + "Height"), answer_height[i]);
        property(engine::ui::intern("answer" + n + "Gap"), answer_gap[i]);
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

void HelpTopicViewModel::note_answer_height(std::size_t index, float height) {
    if (index >= kHelpPredictionsPerTopic || height <= 1.f) {
        return;
    }
    if (std::abs(natural_[index] - height) < 0.5f) {
        return;
    }
    natural_[index] = height;
}

void HelpTopicViewModel::apply_answer_motion() {
    for (std::size_t i = 0; i < kHelpPredictionsPerTopic; ++i) {
        const bool open = revealed_[i] && natural_[i] > 1.f;
        const std::string height = open ? std::format("{:.0f}", std::ceil(natural_[i] + 2.f)) : "0";
        const std::string gap = open ? "10 0 0 0" : "0";
        if (answer_height[i].get() != height) {
            answer_height[i] = height;
        }
        if (answer_gap[i].get() != gap) {
            answer_gap[i] = gap;
        }
    }
}

void HelpTopicViewModel::toggle(std::size_t i) {
    revealed_[i] = !revealed_[i];
    reveal_label[i] = revealed_[i] ? hide_ : show_;
    apply_answer_motion();
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
            &topicBfield,
            &topicLorentz,
            &topicCyclotron,
            &topicExb,
            &topicCoil,
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
            &topicBfieldDisplay,
            &topicLorentzDisplay,
            &topicCyclotronDisplay,
            &topicExbDisplay,
            &topicCoilDisplay,
            &topicGravityDisplay,
            &topicOverviewDisplay,
            &topicSimulationDisplay,
    } {
    assets::ui::Help::bind(*this);
    // Codegen skips scroll-x / scroll-y.
    property(engine::ui::intern("contentScroll"), contentScroll);
    property(engine::ui::intern("navScroll"), navScroll);
}

}
