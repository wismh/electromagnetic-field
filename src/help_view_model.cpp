#include <game/help_view_model.h>

#include <engine/ui/binding_id.h>

#include <asset_ids.h>

namespace game {

// Item view models are registered by hand: their bindings live in included template files, and
// one class serves every nav row / every topic file.
HelpNavItem::HelpNavItem() {
    property(engine::ui::intern("title"), title);
    property(engine::ui::intern("background"), background);
    property(engine::ui::intern("textColor"), textColor);
    command(engine::ui::intern("select"), select);
}

HelpTopicViewModel::HelpTopicViewModel() {
    command(engine::ui::intern("tryIt"), tryIt);
}

HelpViewModel::HelpViewModel() :
    topics{
            &topicCharge,
            &topicCoulomb,
            &topicField,
            &topicSuperposition,
            &topicFieldLines,
            &topicPotential,
            &topicEnergy,
            &topicMotion,
            &topicSoftening,
            &topicCollisions,
            &topicExperiments,
            &topicUnits,
            &topicLimits,
    },
    topic_displays{
            &topicChargeDisplay,
            &topicCoulombDisplay,
            &topicFieldDisplay,
            &topicSuperpositionDisplay,
            &topicFieldLinesDisplay,
            &topicPotentialDisplay,
            &topicEnergyDisplay,
            &topicMotionDisplay,
            &topicSofteningDisplay,
            &topicCollisionsDisplay,
            &topicExperimentsDisplay,
            &topicUnitsDisplay,
            &topicLimitsDisplay,
    } {
    assets::ui::Help::bind(*this);
}

}
