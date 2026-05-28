#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>

#include <game/field_view.h>
#include <game/help_view_model.h>
#include <game/scene.h>

#include <array>
#include <cstddef>
#include <functional>
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace game {

// What a topic's "Try it" button sets up in the simulation.
struct HelpDemo {
    Preset preset = Preset::Dipole;
    FieldLayers layers;
};

struct HelpTopic {
    const char* section;  // nav header shown above the first topic of each section
    const char* title;
    std::optional<HelpDemo> demo;
};

// The reference topics in navigation order; matches HelpViewModel::topics and assets/ui/help_topics/.
[[nodiscard]] std::span<const HelpTopic, kHelpTopicCount> help_topics();

// Full-screen in-app reference (assets/ui/help.xml): a nav list plus one topic file per topic,
// all in one canvas, with inactive topics removed by `display: none`.
class Help {
public:
    using DemoHandler = std::function<void(const HelpDemo&)>;
    using LocaleHandler = std::function<void(std::string_view locale)>;

    Help(engine::ecs::World& world, DemoHandler on_demo, LocaleHandler on_locale);

    // Rewrites nav titles, reveal buttons, and the language-segment highlight from the catalog.
    void apply_locale();

    void open();
    void close();
    void toggle();
    [[nodiscard]] bool is_open() const {
        return open_;
    }

    // `reveal_in_nav` scrolls the nav list so the topic row is visible (keyboard selection).
    void select(std::size_t topic, bool reveal_in_nav = false);
    // Keyboard navigation: previous / next topic, and the selected topic's "try it".
    void select_previous();
    void select_next();
    void try_selected();
    // Centres the help window and caps its width for readable line lengths. Call every frame.
    void update_layout(glm::ivec2 window_size);

private:
    void sync();

    engine::ecs::World& world_;
    engine::ecs::Entity canvas_{};
    std::shared_ptr<HelpViewModel> vm_;
    DemoHandler on_demo_;
    LocaleHandler on_locale_;
    bool open_ = false;
    std::size_t selected_ = 0;
    // Topic index -> row in navItems (section headers sit between topic rows).
    std::array<std::size_t, kHelpTopicCount> nav_rows_{};
    // Topic index -> y of its nav row inside the scrolled list, for reveal_in_nav.
    std::array<float, kHelpTopicCount> nav_row_y_{};
    float nav_list_height_ = 0.f;
    // Visible height of the nav list, derived from the window size in update_layout.
    float nav_view_height_ = 0.f;
};

}
