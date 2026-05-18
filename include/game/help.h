#pragma once

#include <engine/ecs/world.h>

#include <game/field_view.h>
#include <game/help_view_model.h>
#include <game/scene.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>

namespace game {

// What a topic's "Спробуй" button sets up in the simulation.
struct HelpDemo {
    Preset preset = Preset::Dipole;
    FieldLayers layers;
};

struct HelpTopic {
    const char* title;
    std::optional<HelpDemo> demo;
};

// The 13 reference topics in navigation order; matches HelpViewModel::topics and assets/ui/help_topics/.
[[nodiscard]] std::span<const HelpTopic, kHelpTopicCount> help_topics();

// Full-screen in-app reference (assets/ui/help.xml): a nav list plus one topic file per topic,
// all in one canvas, with inactive topics removed by `display: none`.
class Help {
public:
    using DemoHandler = std::function<void(const HelpDemo&)>;

    Help(engine::ecs::World& world, DemoHandler on_demo);

    void open();
    void close();
    void toggle();
    [[nodiscard]] bool is_open() const {
        return open_;
    }

    void select(std::size_t topic);
    // Keyboard navigation: previous / next topic, and the selected topic's "try it".
    void select_previous();
    void select_next();
    void try_selected();

private:
    void sync();

    std::shared_ptr<HelpViewModel> vm_;
    DemoHandler on_demo_;
    bool open_ = false;
    std::size_t selected_ = 0;
};

}
