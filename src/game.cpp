#include <game/game.h>

#include <game/camera_control.h>

#include <engine/core/platform.h>
#include <engine/core/time.h>
#include <engine/ecs/camera.h>
#include <engine/ecs/schedule.h>
#include <engine/ecs/transform.h>
#include <engine/loc/catalog.h>
#include <engine/log.h>
#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <string>

namespace game {
namespace {

// Size after un-maximize. The window opens maximized.
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;

}

Game::Game(const engine::EngineServices& services) :
    assets_(services.assets),
    input_system_(services.input) {}

engine::WindowDesc Game::primary_window() const {
    return {
            .title = "Electromagnetic Field",
            .size = {kWindowW, kWindowH},
            .style = {.resizable = true, .maximized = true},
    };
}

std::optional<engine::AssetId> Game::window_icon() const {
    return assets::textures::icon;
}

void Game::on_start() {
    actions_ = bind_actions(input_system_);
    load_locale();
    spawn_camera();
    charge_view_.emplace(world_, assets_);
    field_view_.emplace(world_, assets_);
    help_.emplace(world_, [this](const HelpDemo& demo) { apply_demo(demo); },
            [this](std::string_view tag) { set_locale(tag); });
    scenes_.emplace(world_, [this](Preset preset) { load_preset(preset); });
    welcome_.emplace(world_, [this](std::string_view tag) { set_locale(tag); },
            [this] { prefs_->set_welcome_seen(); });
    input_.emplace(world_, camera_, sim_, layers_, clock_, new_charge_, panel_visible_, actions_, SandboxHooks{
            .load_preset = [this](Preset preset) { load_preset(preset); },
            .clear = [this] { clear_charges(); },
            .reload = [this] { load_preset(preset_); },
    });
    hud_.emplace(world_, *this, clock_, *help_, *scenes_);
    if (!prefs_->welcome_seen()) {
        welcome_->open();
    }
    load_preset(preset_);
    world_.add_system(engine::ecs::Schedule::Frame, engine::ecs::Phase::Game, [this](engine::ecs::World&) {
        frame_update();
    });
}

void Game::on_fixed_update() {
    if (!clock_.paused || clock_.step_requested) {
        // Sub-step so a high time scale does not take one large dt.
        const float scale = clock_.step_requested ? 1.f : clock_.scale;
        const int substeps = std::max(1, static_cast<int>(std::ceil(scale)));
        const float dt = engine::kFixed * scale / static_cast<float>(substeps);
        for (int i = 0; i < substeps; ++i) {
            sim_.step(dt);
            sim_time_ += dt;
        }
        clock_.step_requested = false;
        assign_ids();
        trails_.record(sim_.charges(), sim_time_);
    }
    GameBase::on_fixed_update();
}

void Game::frame_update() {
    const float dt = world_.ctx<engine::Time>().delta_time;
    help_->tick(dt);
    scenes_->tick(dt);
    welcome_->tick(dt);
    const bool overlays = overlay_open();
    if (overlay_was_open_ && !overlays) {
        input_->assume_pointer_on_ui();
    }
    overlay_was_open_ = overlays;
    {
        const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
        const glm::ivec2 size{window.width, window.height};
        help_->update_layout(size);
        scenes_->update_layout(size);
        welcome_->update_layout(size);
    }
    input_->handle_mouse(overlays);
    handle_keys();
    assign_ids();
    input_->refresh_pointer();
    update_views();
}

void Game::spawn_camera() {
    camera_ = world_.create();
    world_.emplace<engine::Transform>(camera_);
    world_.emplace<engine::Camera>(camera_, engine::Camera{.ortho_size = kDefaultOrthoHalf});
    world_.ctx<engine::ActiveCamera>().entity = camera_;
}

void Game::load_locale() {
    std::filesystem::path dir;
    if (const auto result = engine::user_data_directory("wismh", "electromagnetic-field")) {
        dir = *result;
    } else {
        engine::log::warn(std::format("user data directory unavailable: {}", result.error().message()));
    }
    prefs_.emplace(std::move(dir));
    prefs_->migrate_legacy();

    auto& catalog = world_.ctx<engine::loc::Catalog>();
    const auto english = assets_.get<engine::loc::StringTable>(assets::locale::en);
    const auto ukrainian = assets_.get<engine::loc::StringTable>(assets::locale::uk);
    catalog.add(*english, engine::loc::Role::Source);
    catalog.add(*ukrainian);
    catalog.set_fallback("en");
    catalog.set_active(prefs_->locale());
}

void Game::set_locale(std::string_view tag) {
    if (tag != "en" && tag != "uk") {
        return;
    }
    auto& catalog = world_.ctx<engine::loc::Catalog>();
    if (catalog.active() == tag) {
        return;
    }
    catalog.set_active(std::string(tag));
    prefs_->set_locale(tag);
    help_->apply_locale();
    welcome_->apply_locale();
    update_views();
}

void Game::handle_keys() {
    // Key repeat never becomes an InputEvent.
    for (const engine::KeyEvent& event : engine::ecs::EventReader<engine::KeyEvent>{world_, key_cursor_}) {
        if (!event.down || event.key != engine::KeyCode::Right) {
            continue;
        }
        if (welcome_->is_open() || scenes_->is_open() || help_->is_open()) {
            continue;
        }
        input_->step();
    }
    for (const engine::InputEvent& event : engine::ecs::EventReader<engine::InputEvent>{world_, input_cursor_}) {
        if (event.kind != engine::InputEvent::Kind::Down) {
            continue;
        }
        const engine::ActionId action = event.action;
        if (welcome_->is_open()) {
            if (action == actions_.close) {
                welcome_->close();
            }
            continue;
        }
        if (scenes_->is_open()) {
            if (action == actions_.close) {
                scenes_->close();
            } else if (const std::optional<Preset> preset = actions_.preset_for(action)) {
                scenes_->close();
                load_preset(*preset);
            }
            continue;
        }
        if (action == actions_.help) {
            help_->toggle();
            continue;
        }
        // Help takes up/down, confirm and close.
        if (help_->is_open()) {
            if (action == actions_.close) {
                help_->close();
            } else if (action == actions_.up) {
                help_->select_previous();
            } else if (action == actions_.down) {
                help_->select_next();
            } else if (action == actions_.confirm) {
                help_->try_selected();
            }
            continue;
        }
        input_->on_action(action);
    }
}

void Game::load_preset(Preset preset) {
    input_->end_drag();
    preset_ = preset;
    scene_cleared_ = false;
    sim_.charges() = make_preset(preset);
    input_->clear_hover();
    // Each scene sets its own B; electrostatic scenes clear it.
    const SceneSettings settings = scene_settings(preset);
    FieldParams params = sim_.params();
    params.b_external = settings.b_external;
    sim_.set_params(params);
    DynamicsOptions dynamics = sim_.dynamics();
    dynamics.magnetic = settings.magnetic;
    sim_.set_dynamics(dynamics);
    sim_.coils() = settings.coils;
    // Trails record only while the simulation steps.
    trails_.clear();
}

void Game::apply_demo(const HelpDemo& demo) {
    load_preset(demo.preset);
    layers_ = demo.layers;
    clock_.paused = false;
    clock_.set_scale(1.f);
}

void Game::clear_charges() {
    input_->end_drag();
    sim_.charges().clear();
    sim_.coils().clear();
    input_->clear_hover();
    scene_cleared_ = true;
    trails_.clear();
}

void Game::assign_ids() {
    for (Charge& c : sim_.charges()) {
        if (c.id == 0) {
            c.id = next_id_++;
        }
    }
}

bool Game::overlay_open() const {
    return help_->is_open() || scenes_->is_open() || welcome_->is_open();
}

bool Game::pointer_over_ui() const {
    return input_->pointer_on_ui() || overlay_open();
}

CameraView Game::camera_view() const {
    return CameraView{
            .position = world_.get<engine::Transform>(camera_).position,
            .ortho_half = world_.get<engine::Camera>(camera_).ortho_size,
    };
}

Bounds Game::view_bounds() {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    const CameraView view = camera_view();
    const float aspect = window.height > 0 ? static_cast<float>(window.width) / static_cast<float>(window.height) : 1.f;
    const glm::vec2 half{view.ortho_half * aspect, view.ortho_half};
    const glm::vec2 centre{view.position.x, view.position.y};
    return Bounds{centre - half, centre + half};
}

void Game::update_views() {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    const bool show_probe = layers_.probe && !input_->hovering_charge() && !input_->dragging() && !input_->panning() &&
            !pointer_over_ui();
    if (window.height > 0) {
        field_view_->update(FieldView::Frame{
                .charges = sim_.charges(),
                .coils = sim_.coils(),
                .params = sim_.params(),
                .view = view_bounds(),
                .world_per_pixel = 2.f * camera_view().ortho_half / static_cast<float>(window.height),
                // Real time, including while the simulation is paused.
                .dt = world_.ctx<engine::Time>().delta_time,
                .probe = show_probe ? std::optional<glm::vec3>{input_->pointer_world()} : std::nullopt,
                .trails = &trails_,
                .sim_time = sim_time_,
                .layers = layers_,
                .magnetic_force = sim_.dynamics().magnetic,
        });
    }
    charge_view_->update(ChargeView::Frame{
            .charges = sim_.charges(),
            .params = sim_.params(),
            .camera = camera_view(),
            .show_potential = layers_.potential,
            .hovered = input_->hovered(),
    });
    panel_sync_.sync(hud_->view_model(), sim_, layers_, clock_, new_charge_, PanelFrame{
            .catalog = &world_.ctx<engine::loc::Catalog>(),
            .charges = sim_.charges(),
            .coils = sim_.coils(),
            .preset = preset_,
            .scene_cleared = scene_cleared_,
            .panel_visible = panel_visible_,
            .show_probe = show_probe,
            .pointer_screen = input_->pointer_screen(),
            .pointer_world = input_->pointer_world(),
            .window = window,
    });
    const std::optional<Preset> current = scene_cleared_ ? std::nullopt : std::optional<Preset>{preset_};
    scenes_->set_current(current);
}

}
