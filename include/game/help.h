#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>

#include <game/field_layers.h>
#include <game/help_view_model.h>
#include <game/overlay_pop.h>
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

struct HelpDemo {
    Preset preset = Preset::Dipole;
    FieldLayers layers;
};

struct HelpTopic {
    const char* section;
    const char* title;
    std::optional<HelpDemo> demo;
};

// Same order as HelpViewModel::topics and assets/ui/help_topics/.
[[nodiscard]] std::span<const HelpTopic, kHelpTopicCount> help_topics();

class Help {
public:
    using DemoHandler = std::function<void(const HelpDemo&)>;
    using LocaleHandler = std::function<void(std::string_view locale)>;

    Help(engine::ecs::World& world, DemoHandler on_demo, LocaleHandler on_locale);

    void apply_locale();

    void open();
    void close();
    void toggle();
    void tick(float dt);
    [[nodiscard]] bool is_open() const {
        return pop_.is_open();
    }

    void select(std::size_t topic, bool reveal_in_nav = false);
    void select_previous();
    void select_next();
    void try_selected();
    void update_layout(glm::ivec2 window_size);

private:
    void sync();
    void sample_answers();

    engine::ecs::World& world_;
    engine::ecs::Entity canvas_{};
    std::shared_ptr<HelpViewModel> vm_;
    DemoHandler on_demo_;
    LocaleHandler on_locale_;
    OverlayPop pop_;
    std::size_t selected_ = 0;
    std::array<std::size_t, kHelpTopicCount> nav_rows_{};
    std::array<float, kHelpTopicCount> nav_row_y_{};
    float nav_list_height_ = 0.f;
    float nav_view_height_ = 0.f;
};

}
