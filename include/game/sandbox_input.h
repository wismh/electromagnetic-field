#pragma once

#include <engine/core/input_system.h>
#include <engine/ecs/entity.h>
#include <engine/ecs/events.h>
#include <engine/ecs/world.h>

#include <game/camera_control.h>
#include <game/field_layers.h>
#include <game/input_actions.h>
#include <game/scene.h>
#include <game/sim_clock.h>
#include <game/simulation.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <functional>
#include <optional>

namespace game {

struct SandboxHooks {
    std::function<void(Preset)> load_preset;
    std::function<void()> clear;
    std::function<void()> reload;
};

// drag_'s index stays valid until end_drag; load and clear call end_drag before they resize charges.
class SandboxInput {
public:
    SandboxInput(engine::ecs::World& world, engine::ecs::Entity camera, Simulation& sim, FieldLayers& layers,
            SimClock& clock, float& new_charge, bool& panel_visible, InputActions actions, SandboxHooks hooks);

    void handle_mouse(bool overlays_open);
    void on_action(engine::ActionId action);
    // Key repeat: InputEvent is only the first press.
    void step();
    void refresh_pointer();

    void end_drag();
    void clear_hover();
    void assume_pointer_on_ui();

    [[nodiscard]] bool pointer_on_ui() const;
    [[nodiscard]] bool dragging() const;
    [[nodiscard]] bool panning() const;
    [[nodiscard]] bool hovering_charge() const;
    [[nodiscard]] glm::vec2 pointer_screen() const;
    [[nodiscard]] const glm::vec3& pointer_world() const;
    [[nodiscard]] std::optional<std::size_t> hovered() const;

private:
    struct Drag {
        std::size_t index = 0;
        glm::vec3 offset{0.f};
        bool was_fixed = false;
    };

    void on_mouse_down(engine::MouseButton button);
    void on_mouse_up(engine::MouseButton button);
    void on_mouse_move();
    void on_wheel(float notches);
    void add_charge(float q);
    void remove_charge(std::size_t index);
    void update_hover();
    [[nodiscard]] glm::vec3 pointer_to_world(glm::vec2 screen) const;
    [[nodiscard]] CameraView camera_view() const;
    void set_camera_view(const CameraView& view);
    [[nodiscard]] bool pointer_blocked(bool overlays_open) const;

    engine::ecs::World& world_;
    engine::ecs::Entity camera_;
    Simulation& sim_;
    FieldLayers& layers_;
    SimClock& clock_;
    float& new_charge_;
    bool& panel_visible_;
    InputActions actions_;
    SandboxHooks hooks_;

    engine::ecs::EventCursor<engine::MouseEvent> mouse_cursor_;
    glm::vec2 pointer_screen_{0.f};
    // Latched: MouseConsumed is cleared every frame and set only while routing events.
    bool pointer_on_ui_ = false;
    glm::vec3 pointer_world_{0.f};
    std::optional<glm::vec3> pan_grab_;
    std::optional<std::size_t> hovered_;
    std::optional<Drag> drag_;
};

}
