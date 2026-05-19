#pragma once

#include <engine/core/input_system.h>
#include <engine/ecs/entity.h>
#include <engine/ecs/events.h>
#include <engine/igame.h>
#include <engine/resources/assets_db.h>

#include <game/camera_control.h>
#include <game/field_view.h>
#include <game/help.h>
#include <game/panel_view_model.h>
#include <game/scene.h>
#include <game/simulation.h>
#include <game/trails.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace game {

class Game final : public engine::GameBase {
public:
    explicit Game(engine::AssetsDb& assets);

    engine::WindowDesc primary_window() const override;

    void on_start() override;
    void on_fixed_update() override;

private:
    struct Drag {
        std::size_t index = 0;
        glm::vec3 offset{0.f};
        bool was_fixed = false;
    };

    // Runs as a Frame/Game-phase system, i.e. after the engine's Input phase has routed this frame's
    // mouse events through the UI, so MouseConsumed already says whether the pointer is over the panel.
    void frame_update();
    void spawn_camera();
    void spawn_panel();
    void bind_panel_commands();
    void sync_panel();
    [[nodiscard]] bool pointer_over_ui();
    void spawn_potential_layer();
    [[nodiscard]] CameraView camera_view();
    void set_camera_view(const CameraView& view);
    void follow_camera();

    void handle_mouse();
    void handle_keys();
    void on_mouse_down(engine::MouseButton button);
    void on_mouse_up(engine::MouseButton button);
    void on_mouse_move();
    void on_wheel(float notches);
    void on_key(engine::KeyCode key);

    void load_preset(Preset preset);
    void apply_demo(const HelpDemo& demo);
    void clear_charges();
    void set_time_scale(float scale);
    void add_charge(float q);
    void remove_charge(std::size_t index);
    void end_drag();
    void assign_ids();
    [[nodiscard]] glm::vec3 pointer_to_world(glm::vec2 screen);
    void update_hover();

    [[nodiscard]] Bounds view_bounds();
    void update_field_view();
    void sync_charge_views();
    void sync_potential_uniforms();

    engine::AssetsDb& assets_;
    Simulation sim_;
    Trails trails_;
    float sim_time_ = 0.f;
    std::uint32_t next_id_ = 1;
    Preset preset_ = Preset::Dipole;

    engine::ecs::Entity camera_{};
    engine::ecs::Entity potential_layer_{};
    std::vector<engine::ecs::Entity> charge_views_;
    // Built in on_start, once the asset catalog is loaded.
    std::optional<FieldView> field_view_;
    FieldLayers layers_;
    std::shared_ptr<PanelViewModel> panel_;
    // Built in on_start, after the panel (its canvas draws above it).
    std::optional<Help> help_;
    bool panel_visible_ = true;
    // Last values written into the panel's two-way fields; a difference means the user changed them.
    struct PanelEcho {
        FieldLayers layers;
        bool collisions = true;
        float time_frac = -1.f;
        float k_frac = -1.f;
        float eps_frac = -1.f;
        float new_charge_frac = -1.f;
    } panel_echo_;
    // False until the first sync has written the game state into the panel; until then the panel's
    // default-constructed fields are not user input.
    bool panel_synced_ = false;
    float new_charge_ = 1.f;
    bool scene_cleared_ = false;

    engine::ecs::EventCursor<engine::MouseEvent> mouse_cursor_;
    engine::ecs::EventCursor<engine::KeyEvent> key_cursor_;
    glm::vec2 pointer_screen_{0.f};
    // Whether the pointer is over a hit-testable UI element. Latched from MouseConsumed on frames
    // that have mouse events: the engine clears MouseConsumed every frame and only re-sets it while
    // routing events, so on an idle frame it would wrongly read "not over UI".
    bool pointer_on_ui_ = false;
    // Help open state last frame: when it closes, pointer_on_ui_ still describes the help screen,
    // so it is assumed "over UI" until the next mouse event re-latches it.
    bool help_was_open_ = false;
    glm::vec3 pointer_world_{0.f};
    // World point grabbed by a middle-button pan; the camera moves so it stays under the cursor.
    std::optional<glm::vec3> pan_grab_;
    std::optional<std::size_t> hovered_;
    std::optional<Drag> drag_;

    bool paused_ = false;
    bool step_requested_ = false;
    float time_scale_ = 1.f;
};

}
