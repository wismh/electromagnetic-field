#pragma once

#include <engine/core/input_system.h>
#include <engine/ecs/entity.h>
#include <engine/ecs/events.h>
#include <engine/igame.h>
#include <engine/resources/assets_db.h>

#include <game/scene.h>
#include <game/simulation.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <optional>
#include <vector>

namespace game {

class Game final : public engine::GameBase {
public:
    explicit Game(engine::AssetsDb& assets);

    engine::WindowDesc primary_window() const override;

    void on_start() override;
    void on_fixed_update() override;
    void on_update() override;

private:
    struct Drag {
        std::size_t index = 0;
        glm::vec3 offset{0.f};
        bool was_fixed = false;
    };

    void spawn_camera();
    void spawn_potential_layer();

    void handle_mouse();
    void handle_keys();
    void on_mouse_down(engine::MouseButton button);
    void on_mouse_up(engine::MouseButton button);
    void on_mouse_move();
    void on_wheel(float notches);
    void on_key(engine::KeyCode key);

    void load_preset(Preset preset);
    void add_charge(float q);
    void remove_charge(std::size_t index);
    void end_drag();
    [[nodiscard]] glm::vec3 pointer_to_world(glm::vec2 screen);
    void update_hover();

    void sync_charge_views();
    void sync_potential_uniforms();

    engine::AssetsDb& assets_;
    Simulation sim_;
    Preset preset_ = Preset::Dipole;

    engine::ecs::Entity camera_{};
    engine::ecs::Entity potential_layer_{};
    std::vector<engine::ecs::Entity> charge_views_;

    engine::ecs::EventCursor<engine::MouseEvent> mouse_cursor_;
    engine::ecs::EventCursor<engine::KeyEvent> key_cursor_;
    glm::vec3 pointer_world_{0.f};
    std::optional<std::size_t> hovered_;
    std::optional<Drag> drag_;

    bool paused_ = false;
    bool step_requested_ = false;
    float time_scale_ = 1.f;
};

}
