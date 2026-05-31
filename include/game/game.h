#pragma once

#include <engine/core/engine_services.h>
#include <engine/core/input_system.h>
#include <engine/ecs/entity.h>
#include <engine/ecs/events.h>
#include <engine/igame.h>
#include <engine/resources/assets_db.h>

#include <game/charge_view.h>
#include <game/field_layers.h>
#include <game/field_view.h>
#include <game/help.h>
#include <game/input_actions.h>
#include <game/panel_hud.h>
#include <game/panel_sync.h>
#include <game/sandbox_input.h>
#include <game/scene.h>
#include <game/scene_gallery.h>
#include <game/sim_clock.h>
#include <game/simulation.h>
#include <game/trails.h>
#include <game/user_prefs.h>
#include <game/welcome.h>

#include <cstdint>
#include <optional>
#include <string_view>

namespace game {

class Game final : public engine::GameBase {
    friend class PanelHud;

public:
    explicit Game(const engine::EngineServices& services);

    engine::WindowDesc primary_window() const override;
    std::optional<engine::AssetId> window_icon() const override;

    void on_start() override;
    void on_fixed_update() override;

private:
    // After Input, so MouseConsumed already reflects the panel.
    void frame_update();
    void spawn_camera();
    void load_locale();
    void set_locale(std::string_view tag);
    void handle_keys();
    void load_preset(Preset preset);
    void apply_demo(const HelpDemo& demo);
    void clear_charges();
    void assign_ids();

    [[nodiscard]] bool overlay_open() const;
    [[nodiscard]] bool pointer_over_ui() const;
    [[nodiscard]] CameraView camera_view() const;
    // window_size_for takes a non-const World.
    [[nodiscard]] Bounds view_bounds();
    void update_views();

    engine::AssetsDb& assets_;
    engine::InputSystem& input_system_;
    InputActions actions_;
    Simulation sim_;
    Trails trails_;
    float sim_time_ = 0.f;
    std::uint32_t next_id_ = 1;
    Preset preset_ = Preset::Dipole;
    SimClock clock_;
    FieldLayers layers_;
    float new_charge_ = 1.f;
    bool panel_visible_ = true;
    bool scene_cleared_ = false;

    engine::ecs::Entity camera_{};
    // Set in on_start; never cleared.
    std::optional<FieldView> field_view_;
    std::optional<ChargeView> charge_view_;
    std::optional<Help> help_;
    std::optional<SceneGallery> scenes_;
    std::optional<Welcome> welcome_;
    std::optional<SandboxInput> input_;
    std::optional<UserPrefs> prefs_;
    std::optional<PanelHud> hud_;
    PanelSync panel_sync_;

    engine::ecs::EventCursor<engine::KeyEvent> key_cursor_;
    engine::ecs::EventCursor<engine::InputEvent> input_cursor_;
    // Stale until the next mouse event after an overlay closes.
    bool overlay_was_open_ = false;
};

}
