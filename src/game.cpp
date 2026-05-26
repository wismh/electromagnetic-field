#include <game/game.h>

#include <game/magnetism.h>
#include <game/slider.h>

#include <engine/builtin_ids.h>
#include <engine/core/time.h>
#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>
#include <engine/loc/catalog.h>
#include <engine/log.h>
#include <engine/render/renderable.h>
#include <engine/ecs/schedule.h>
#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <span>
#include <string>

namespace game {
namespace {

// Restored size. The window opens maximized; this is what it returns to.
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
constexpr float kOrthoHalf = 10.f;
// The background quad follows the camera and spans this many view heights, enough for any sane aspect.
constexpr float kPotentialLayerViewHeights = 4.f;
// Must match kQuadRadii in assets/shaders/charge.shader.
constexpr float kChargeQuadRadii = 3.f;

constexpr float kMinTimeScale = 0.125f;
constexpr float kMaxTimeScale = 8.f;

constexpr glm::vec4 kPositiveColor{1.f, 0.42f, 0.2f, 1.f};
constexpr glm::vec4 kNegativeColor{0.25f, 0.55f, 1.f, 1.f};

// The time slider works on log2(scale) so that halving and doubling are equal distances.
constexpr SliderRange kTimeScaleLog2Range{.min = -3.f, .max = 3.f, .step = 0.25f};
constexpr SliderRange kCoulombRange{.min = 0.1f, .max = 5.f, .step = 0.05f};
constexpr SliderRange kSofteningRange{.min = 0.05f, .max = 1.f, .step = 0.01f};
constexpr SliderRange kNewChargeRange{.min = kMinAbsCharge, .max = kMaxAbsCharge, .step = kChargeStep};
// Signed and linear, so 0 sits in the middle of the track.
constexpr SliderRange kExternalBRange{.min = -2.f, .max = 2.f, .step = 0.05f};
// The c slider works on log10(c): 3 ... 100. Low c exaggerates magnetism between moving charges.
constexpr SliderRange kLightSpeedLog10Range{.min = 0.48f, .max = 2.f, .step = 0.02f};

// Probe readout offset from the cursor, and its rough size for keeping it on screen.
constexpr float kProbeOffsetPx = 18.f;
constexpr float kProbeBoxW = 170.f;
constexpr float kProbeBoxH = 64.f;
constexpr float kProbeBoxHMagnetic = 84.f;
constexpr float kPanelHiddenRight = -320.f;

const char* preset_key(Preset preset) {
    switch (preset) {
        case Preset::Dipole:
            return "preset.dipole";
        case Preset::LikePair:
            return "preset.like_pair";
        case Preset::Quadrupole:
            return "preset.quadrupole";
        case Preset::Capacitor:
            return "preset.capacitor";
        case Preset::Orbit:
            return "preset.orbit";
        case Preset::Rutherford:
            return "preset.rutherford";
        case Preset::Swarm:
            return "preset.swarm";
        case Preset::Cyclotron:
            return "preset.cyclotron";
        case Preset::ExBDrift:
            return "preset.exb_drift";
        case Preset::Coil:
            return "preset.coil";
    }
    return "preset.dipole";
}

constexpr const char* kLocaleOnBg = "#6366f133";
constexpr const char* kLocaleOnFg = "#ffffff";
constexpr const char* kLocaleOffBg = "#ffffff14";
constexpr const char* kLocaleOffFg = "#aab1c3";

std::filesystem::path locale_file() {
    char* appdata = nullptr;
    size_t length = 0;
    if (_dupenv_s(&appdata, &length, "APPDATA") != 0 || appdata == nullptr || appdata[0] == '\0') {
        std::free(appdata);
        return {};
    }
    const std::filesystem::path path = std::filesystem::path(appdata) / "electromagnetic-field" / "locale.txt";
    std::free(appdata);
    return path;
}

std::string saved_locale() {
    const std::filesystem::path path = locale_file();
    if (path.empty()) {
        return "en";
    }
    std::ifstream in(path);
    std::string tag;
    if (in >> tag && (tag == "en" || tag == "uk")) {
        return tag;
    }
    return "en";
}

void save_locale(std::string_view tag) {
    const std::filesystem::path path = locale_file();
    if (path.empty()) {
        return;
    }
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (out) {
        out << tag << '\n';
    }
}

std::filesystem::path welcome_file() {
    const std::filesystem::path locale = locale_file();
    if (locale.empty()) {
        return {};
    }
    return locale.parent_path() / "welcome.txt";
}

bool welcome_seen() {
    const std::filesystem::path path = welcome_file();
    if (path.empty()) {
        return false;
    }
    std::ifstream in(path);
    std::string flag;
    return static_cast<bool>(in >> flag) && flag == "seen";
}

void save_welcome_seen() {
    const std::filesystem::path path = welcome_file();
    if (path.empty()) {
        return;
    }
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (out) {
        out << "seen\n";
    }
}

std::string catalog_text(const engine::loc::Catalog& catalog, std::string_view key) {
    return catalog.text(key).text;
}

// Powers of two read as \times 1/4, \times 2; the in-between slider stops as decimals.
// A slash, not a stacked fraction: the status and the slider row are one line tall.
// The leading thin space is the gap in front of the formula. Text before \(\) is measured
// by ink, so a word space there would not show.
std::string time_scale_text(float scale) {
    const float octaves = std::log2(scale);
    if (std::abs(octaves - std::round(octaves)) > 1e-3f) {
        return std::format("\\(\\ \\times {:.2g}\\)", scale);
    }
    if (scale < 1.f) {
        return std::format("\\(\\ \\times 1/{}\\)", static_cast<int>(std::lround(1.f / scale)));
    }
    return std::format("\\(\\ \\times {}\\)", static_cast<int>(std::lround(scale)));
}

std::string percent(float fraction) {
    return std::format("{:.1f}%", 100.f * std::clamp(fraction, 0.f, 1.f));
}

}

Game::Game(engine::AssetsDb& assets) :
    assets_(assets) {}

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
    load_locale();
    spawn_camera();
    spawn_potential_layer();
    field_view_.emplace(world_, assets_);
    spawn_panel();
    help_.emplace(world_, [this](const HelpDemo& demo) { apply_demo(demo); },
            [this](std::string_view tag) { set_locale(tag); });
    welcome_.emplace(world_, [this](std::string_view tag) { set_locale(tag); }, [this] { save_welcome_seen(); });
    if (!welcome_seen()) {
        welcome_->open();
    }
    load_preset(preset_);
    world_.add_system(engine::ecs::Schedule::Frame, engine::ecs::Phase::Game, [this](engine::ecs::World&) {
        frame_update();
    });
}

void Game::on_fixed_update() {
    if (!paused_ || step_requested_) {
        // Sub-step so a high time scale does not mean a large, unstable dt.
        const float scale = step_requested_ ? 1.f : time_scale_;
        const int substeps = std::max(1, static_cast<int>(std::ceil(scale)));
        const float dt = engine::kFixed * scale / static_cast<float>(substeps);
        for (int i = 0; i < substeps; ++i) {
            sim_.step(dt);
            sim_time_ += dt;
        }
        step_requested_ = false;
        assign_ids();
        trails_.record(sim_.charges(), sim_time_);
    }
    GameBase::on_fixed_update();
}

void Game::frame_update() {
    const bool overlay_open = help_->is_open() || welcome_->is_open();
    if (overlay_was_open_ && !overlay_open) {
        pointer_on_ui_ = true;
    }
    overlay_was_open_ = overlay_open;
    {
        const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
        const glm::ivec2 size{window.width, window.height};
        help_->update_layout(size);
        welcome_->update_layout(size);
    }
    handle_mouse();
    handle_keys();
    assign_ids();
    // The camera may have moved (pan, zoom, reset), so re-derive the world point under the cursor.
    pointer_world_ = pointer_to_world(pointer_screen_);
    on_mouse_move();
    update_hover();
    follow_camera();
    update_field_view();
    sync_charge_views();
    sync_potential_uniforms();
    sync_panel();
}

void Game::spawn_camera() {
    camera_ = world_.create();
    world_.emplace<engine::Transform>(camera_);
    world_.emplace<engine::Camera>(camera_, engine::Camera{.ortho_size = kOrthoHalf});
    world_.ctx<engine::ActiveCamera>().entity = camera_;
}

void Game::spawn_potential_layer() {
    potential_layer_ = world_.create();
    world_.emplace<engine::Transform>(potential_layer_, engine::Transform{
    });
    world_.emplace<engine::render::Renderable>(potential_layer_, engine::render::Renderable{
            .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
            .material = assets_.get<engine::render::IMaterial>(assets::materials::potential),
            .layer = -100,
            .material_override = engine::render::MaterialOverride{},
    });
}

CameraView Game::camera_view() {
    return CameraView{
            .position = world_.get<engine::Transform>(camera_).position,
            .ortho_half = world_.get<engine::Camera>(camera_).ortho_size,
    };
}

void Game::set_camera_view(const CameraView& view) {
    world_.get<engine::Transform>(camera_).position = view.position;
    world_.get<engine::Camera>(camera_).ortho_size = view.ortho_half;
}

void Game::follow_camera() {
    const CameraView view = camera_view();
    // Hidden by collapsing the quad: Renderable has no visibility flag.
    const float size = layers_.potential ? 2.f * view.ortho_half * kPotentialLayerViewHeights : 0.f;
    auto& transform = world_.get<engine::Transform>(potential_layer_);
    transform.position = {view.position.x, view.position.y, 0.f};
    transform.scale = {size, size, 1.f};
}

void Game::handle_mouse() {
    for (const engine::MouseEvent& event : engine::ecs::EventReader<engine::MouseEvent>{world_, mouse_cursor_}) {
        if (event.window != engine::kPrimaryWindow) {
            continue;
        }
        // The UI already routed all of this frame's events, so MouseConsumed is the final hover
        // state for the frame. Up events never set it (only Move/Down/Wheel hit-test), so a frame
        // holding just a button release must not overwrite the latched value.
        if (event.kind != engine::MouseEvent::Kind::Up) {
            pointer_on_ui_ = world_.ctx<engine::ui::MouseConsumed>().consumed_for(engine::kPrimaryWindow);
        }
        pointer_screen_ = event.position;
        switch (event.kind) {
            case engine::MouseEvent::Kind::Move:
                if (pan_grab_) {
                    CameraView view = camera_view();
                    view.position += *pan_grab_ - pointer_to_world(event.position);
                    set_camera_view(view);
                }
                pointer_world_ = pointer_to_world(event.position);
                on_mouse_move();
                break;
            case engine::MouseEvent::Kind::Down:
                if (pointer_over_ui()) {
                    break;
                }
                pointer_world_ = pointer_to_world(event.position);
                update_hover();
                on_mouse_down(event.button);
                break;
            case engine::MouseEvent::Kind::Up:
                pointer_world_ = pointer_to_world(event.position);
                on_mouse_up(event.button);
                break;
            case engine::MouseEvent::Kind::Wheel:
                if (pointer_over_ui()) {
                    break;
                }
                pointer_world_ = pointer_to_world(event.position);
                update_hover();
                on_wheel(event.wheel_y);
                break;
        }
    }
}

void Game::handle_keys() {
    for (const engine::KeyEvent& event : engine::ecs::EventReader<engine::KeyEvent>{world_, key_cursor_}) {
        if (!event.down) {
            continue;
        }
        // Auto-repeat only for the single-step key, so holding it scrubs forward.
        if (event.repeat && event.key != engine::KeyCode::Right) {
            continue;
        }
        on_key(event.key);
    }
}

// LMB: grab a charge or place +1. RMB: delete a charge or place -1. MMB: pan the camera.
void Game::on_mouse_down(engine::MouseButton button) {
    if (button == engine::MouseButton::Middle) {
        pan_grab_ = pointer_world_;
        return;
    }
    if (drag_) {
        return;
    }
    if (button == engine::MouseButton::Left) {
        if (hovered_) {
            Charge& c = sim_.charges()[*hovered_];
            drag_ = Drag{.index = *hovered_, .offset = c.position - pointer_world_, .was_fixed = c.fixed};
            // Held charges act as fixed so the integrator does not fight the cursor.
            c.fixed = true;
            c.velocity = glm::vec3{0.f};
        } else {
            add_charge(new_charge_);
        }
    } else if (button == engine::MouseButton::Right) {
        if (hovered_) {
            remove_charge(*hovered_);
        } else {
            add_charge(-new_charge_);
        }
    }
}

void Game::on_mouse_up(engine::MouseButton button) {
    if (button == engine::MouseButton::Left) {
        end_drag();
    } else if (button == engine::MouseButton::Middle) {
        pan_grab_.reset();
    }
}

void Game::on_mouse_move() {
    if (!drag_) {
        return;
    }
    Charge& c = sim_.charges()[drag_->index];
    c.position = pointer_world_ + drag_->offset;
    c.position.z = 0.f;
}

// Over a charge the wheel changes |q|; anywhere else it zooms about the cursor.
void Game::on_wheel(float notches) {
    if (hovered_) {
        Charge& c = sim_.charges()[*hovered_];
        c.q = step_charge_magnitude(c.q, notches);
        return;
    }
    set_camera_view(zoom_about(camera_view(), pointer_world_, notches));
}

void Game::on_key(engine::KeyCode key) {
    using engine::KeyCode;
    if (welcome_->is_open()) {
        if (key == KeyCode::Escape) {
            welcome_->close();
        }
        return;
    }
    if (key == KeyCode::H) {
        help_->toggle();
        return;
    }
    // While the reference is open, the scene shortcuts are off: Up/Down pick a topic, Return runs
    // its "try it", Esc closes.
    if (help_->is_open()) {
        switch (key) {
            case KeyCode::Escape:
                help_->close();
                break;
            case KeyCode::Up:
                help_->select_previous();
                break;
            case KeyCode::Down:
                help_->select_next();
                break;
            case KeyCode::Return:
                help_->try_selected();
                break;
            default:
                break;
        }
        return;
    }
    switch (key) {
        case KeyCode::Space:
            paused_ = !paused_;
            return;
        case KeyCode::Tab:
            panel_visible_ = !panel_visible_;
            return;
        case KeyCode::Right:
            if (paused_) {
                step_requested_ = true;
            }
            return;
        case KeyCode::Up:
            set_time_scale(time_scale_ * 2.f);
            return;
        case KeyCode::Down:
            set_time_scale(time_scale_ * 0.5f);
            return;
        case KeyCode::F1:
            layers_.potential = !layers_.potential;
            return;
        case KeyCode::F2:
            layers_.lines = !layers_.lines;
            return;
        case KeyCode::F3:
            layers_.grid = !layers_.grid;
            return;
        case KeyCode::F4:
            layers_.flow = !layers_.flow;
            return;
        case KeyCode::F5:
            layers_.probe = !layers_.probe;
            return;
        case KeyCode::F6:
            layers_.trails = !layers_.trails;
            return;
        case KeyCode::F7:
            layers_.magnetic = !layers_.magnetic;
            return;
        case KeyCode::K: {
            DynamicsOptions dynamics = sim_.dynamics();
            dynamics.collisions = !dynamics.collisions;
            sim_.set_dynamics(dynamics);
            return;
        }
        case KeyCode::M: {
            DynamicsOptions dynamics = sim_.dynamics();
            dynamics.magnetic = !dynamics.magnetic;
            sim_.set_dynamics(dynamics);
            return;
        }
        case KeyCode::Home:
            set_camera_view(CameraView{.position = {0.f, 0.f, 0.f}, .ortho_half = kOrthoHalf});
            return;
        case KeyCode::C:
            clear_charges();
            return;
        case KeyCode::R:
            load_preset(preset_);
            return;
        case KeyCode::Digit1:
            load_preset(Preset::Dipole);
            return;
        case KeyCode::Digit2:
            load_preset(Preset::LikePair);
            return;
        case KeyCode::Digit3:
            load_preset(Preset::Quadrupole);
            return;
        case KeyCode::Digit4:
            load_preset(Preset::Capacitor);
            return;
        case KeyCode::Digit5:
            load_preset(Preset::Orbit);
            return;
        case KeyCode::Digit6:
            load_preset(Preset::Rutherford);
            return;
        case KeyCode::Digit7:
            load_preset(Preset::Swarm);
            return;
        case KeyCode::Digit8:
            load_preset(Preset::Cyclotron);
            return;
        case KeyCode::Digit9:
            load_preset(Preset::ExBDrift);
            return;
        case KeyCode::Digit0:
            load_preset(Preset::Coil);
            return;
        default:
            break;
    }

    // The remaining keys act on the charge under the cursor.
    if (!hovered_) {
        return;
    }
    Charge& c = sim_.charges()[*hovered_];
    switch (key) {
        case KeyCode::L:
            if (drag_ && drag_->index == *hovered_) {
                drag_->was_fixed = !drag_->was_fixed;
            } else {
                c.fixed = !c.fixed;
                c.velocity = glm::vec3{0.f};
            }
            return;
        case KeyCode::F:
            c.q = -c.q;
            return;
        case KeyCode::Delete:
        case KeyCode::Backspace:
            remove_charge(*hovered_);
            return;
        default:
            return;
    }
}

void Game::load_preset(Preset preset) {
    end_drag();
    preset_ = preset;
    scene_cleared_ = false;
    sim_.charges() = make_preset(preset);
    hovered_.reset();
    // Each scene brings its own magnetic setup, so a magnetic scene always shows its effect and an
    // electrostatic one never inherits a field from the previous scene.
    const SceneSettings settings = scene_settings(preset);
    FieldParams params = sim_.params();
    params.b_external = settings.b_external;
    sim_.set_params(params);
    DynamicsOptions dynamics = sim_.dynamics();
    dynamics.magnetic = settings.magnetic;
    sim_.set_dynamics(dynamics);
    sim_.coils() = settings.coils;
}

// A help topic's "try it": its scene with the layers that illustrate it, running at normal speed.
void Game::apply_demo(const HelpDemo& demo) {
    load_preset(demo.preset);
    layers_ = demo.layers;
    paused_ = false;
    set_time_scale(1.f);
}

void Game::clear_charges() {
    end_drag();
    sim_.charges().clear();
    sim_.coils().clear();
    hovered_.reset();
    scene_cleared_ = true;
}

void Game::set_time_scale(float scale) {
    time_scale_ = std::clamp(scale, kMinTimeScale, kMaxTimeScale);
}

void Game::add_charge(float q) {
    if (sim_.charges().size() >= kMaxCharges) {
        engine::log::warn(std::format("charge limit reached ({})", kMaxCharges));
        return;
    }
    sim_.charges().push_back(Charge{.position = {pointer_world_.x, pointer_world_.y, 0.f}, .q = q});
}

void Game::remove_charge(std::size_t index) {
    end_drag();
    auto& charges = sim_.charges();
    charges.erase(charges.begin() + static_cast<std::ptrdiff_t>(index));
    hovered_.reset();
}

// New charges (placed, or loaded from a preset) get the next free id; trails are keyed by it.
void Game::assign_ids() {
    for (Charge& c : sim_.charges()) {
        if (c.id == 0) {
            c.id = next_id_++;
        }
    }
}

void Game::end_drag() {
    if (!drag_) {
        return;
    }
    Charge& c = sim_.charges()[drag_->index];
    c.fixed = drag_->was_fixed;
    c.velocity = glm::vec3{0.f};
    drag_.reset();
}

glm::vec3 Game::pointer_to_world(glm::vec2 screen) {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    if (window.width <= 0 || window.height <= 0) {
        return pointer_world_;
    }
    glm::vec3 p = engine::screen_to_world(
            screen, world_.get<engine::Camera>(camera_), world_.get<engine::Transform>(camera_), window);
    p.z = 0.f;
    return p;
}

void Game::update_hover() {
    if (drag_) {
        hovered_ = drag_->index;
        return;
    }
    hovered_ = pick_charge(sim_.charges(), pointer_world_);
}

Bounds Game::view_bounds() {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    const CameraView view = camera_view();
    const float aspect = window.height > 0 ? static_cast<float>(window.width) / static_cast<float>(window.height) : 1.f;
    const glm::vec2 half{view.ortho_half * aspect, view.ortho_half};
    const glm::vec2 centre{view.position.x, view.position.y};
    return Bounds{centre - half, centre + half};
}

void Game::update_field_view() {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    if (window.height <= 0) {
        return;
    }
    // The probe hides while the cursor is over (or dragging) a charge, where the field is dominated
    // by that charge itself.
    std::optional<glm::vec3> probe;
    if (!hovered_ && !drag_ && !pan_grab_ && !pointer_over_ui() && !help_->is_open() && !welcome_->is_open()) {
        probe = pointer_world_;
    }
    field_view_->update(FieldView::Frame{
            .charges = sim_.charges(),
            .coils = sim_.coils(),
            .params = sim_.params(),
            .view = view_bounds(),
            .world_per_pixel = 2.f * camera_view().ortho_half / static_cast<float>(window.height),
            // Flow tracers visualise the current field, so they keep moving in real time even while
            // the simulation is paused.
            .dt = world_.ctx<engine::Time>().delta_time,
            .probe = probe,
            .trails = &trails_,
            .sim_time = sim_time_,
            .layers = layers_,
            .magnetic_force = sim_.dynamics().magnetic,
    });
}

// One quad entity per charge, created/destroyed as the charge count changes and refreshed every frame.
void Game::sync_charge_views() {
    const auto& charges = sim_.charges();
    while (charge_views_.size() < charges.size()) {
        const engine::ecs::Entity view = world_.create();
        world_.emplace<engine::Transform>(view);
        world_.emplace<engine::render::Renderable>(view, engine::render::Renderable{
                .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
                .material = assets_.get<engine::render::IMaterial>(assets::materials::charge),
                .material_override = engine::render::MaterialOverride{},
        });
        charge_views_.push_back(view);
    }
    while (charge_views_.size() > charges.size()) {
        world_.destroy(charge_views_.back());
        charge_views_.pop_back();
    }

    for (std::size_t i = 0; i < charges.size(); ++i) {
        const Charge& c = charges[i];
        const float size = 2.f * kChargeQuadRadii * charge_radius(c.q);
        auto& transform = world_.get<engine::Transform>(charge_views_[i]);
        transform.position = c.position;
        transform.scale = {size, size, 1.f};

        auto& renderable = world_.get<engine::render::Renderable>(charge_views_[i]);
        renderable.color = c.q >= 0.f ? kPositiveColor : kNegativeColor;
        // Hovered charge draws on top of its neighbours.
        renderable.order_in_layer = hovered_ == i ? 1 : 0;
        renderable.material_override->set_vec4("uStyle", {
                c.q >= 0.f ? 1.f : -1.f,
                c.fixed ? 1.f : 0.f,
                hovered_ == i ? 1.f : 0.f,
                0.f,
        });
    }
}

void Game::sync_potential_uniforms() {
    auto& renderable = world_.get<engine::render::Renderable>(potential_layer_);
    engine::render::MaterialOverride& override = *renderable.material_override;
    const auto& charges = sim_.charges();
    const std::size_t count = std::min(charges.size(), kMaxCharges);
    for (std::size_t i = 0; i < count; ++i) {
        const Charge& c = charges[i];
        override.set_vec4("uCharges[" + std::to_string(i) + "]", {c.position.x, c.position.y, c.q, 0.f});
    }
    const FieldParams& params = sim_.params();
    override.set_vec4("uParams", {static_cast<float>(count), params.k, params.softening, 0.f});
}

// The open reference covers the whole window, so it owns the pointer everywhere.
bool Game::pointer_over_ui() {
    return pointer_on_ui_ || help_->is_open() || welcome_->is_open();
}

void Game::load_locale() {
    auto& catalog = world_.ctx<engine::loc::Catalog>();
    const auto english = assets_.get<engine::loc::StringTable>(assets::locale::en);
    const auto ukrainian = assets_.get<engine::loc::StringTable>(assets::locale::uk);
    catalog.add(*english, engine::loc::Role::Source);
    catalog.add(*ukrainian);
    catalog.set_fallback("en");
    catalog.set_active(saved_locale());
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
    save_locale(tag);
    if (help_) {
        help_->apply_locale();
    }
    if (welcome_) {
        welcome_->apply_locale();
    }
    if (panel_) {
        sync_panel();
    }
}

void Game::paint_panel_locale() {
    const bool english = world_.ctx<engine::loc::Catalog>().active() == "en";
    PanelViewModel& vm = *panel_;
    vm.localeEnBg = english ? kLocaleOnBg : kLocaleOffBg;
    vm.localeEnFg = english ? kLocaleOnFg : kLocaleOffFg;
    vm.localeUkBg = english ? kLocaleOffBg : kLocaleOnBg;
    vm.localeUkFg = english ? kLocaleOffFg : kLocaleOnFg;
}

void Game::spawn_panel() {
    panel_ = std::make_shared<PanelViewModel>();
    bind_panel_commands();
    const engine::ecs::Entity canvas = world_.create();
    world_.emplace<engine::ui::UiCanvas>(canvas, engine::ui::UiCanvas{
            .document = assets::ui::panel,
            .stylesheet = assets::css::panel,
            .data_context = panel_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = 10,
    });
}

void Game::bind_panel_commands() {
    PanelViewModel& vm = *panel_;
    vm.togglePause = [this] { paused_ = !paused_; };
    vm.step = [this] {
        paused_ = true;
        step_requested_ = true;
    };
    vm.presetDipole = [this] { load_preset(Preset::Dipole); };
    vm.presetLikePair = [this] { load_preset(Preset::LikePair); };
    vm.presetQuadrupole = [this] { load_preset(Preset::Quadrupole); };
    vm.presetCapacitor = [this] { load_preset(Preset::Capacitor); };
    vm.presetOrbit = [this] { load_preset(Preset::Orbit); };
    vm.presetRutherford = [this] { load_preset(Preset::Rutherford); };
    vm.presetSwarm = [this] { load_preset(Preset::Swarm); };
    vm.presetCyclotron = [this] { load_preset(Preset::Cyclotron); };
    vm.presetExBDrift = [this] { load_preset(Preset::ExBDrift); };
    vm.presetCoil = [this] { load_preset(Preset::Coil); };
    vm.clearAll = [this] { clear_charges(); };
    vm.openHelp = [this] { help_->open(); };
    vm.localeEn = [this] { set_locale("en"); };
    vm.localeUk = [this] { set_locale("uk"); };
    paint_panel_locale();
}

// Two-way sync with the panel. Toggles and sliders the user changed since the last frame are
// applied to the game first (detected by comparing against the value written last frame); then the
// game state, which keyboard shortcuts may also have changed, is written back for display.
void Game::sync_panel() {
    PanelViewModel& vm = *panel_;

    const bool pull = panel_synced_;
    panel_synced_ = true;
    const auto pull_toggle = [pull](const engine::ui::Bindable<bool>& field, bool echo, bool& target) {
        if (pull && field.get() != echo) {
            target = field.get();
        }
    };
    pull_toggle(vm.showPotential, panel_echo_.layers.potential, layers_.potential);
    pull_toggle(vm.showLines, panel_echo_.layers.lines, layers_.lines);
    pull_toggle(vm.showGrid, panel_echo_.layers.grid, layers_.grid);
    pull_toggle(vm.showFlow, panel_echo_.layers.flow, layers_.flow);
    pull_toggle(vm.showProbe, panel_echo_.layers.probe, layers_.probe);
    pull_toggle(vm.showTrails, panel_echo_.layers.trails, layers_.trails);
    pull_toggle(vm.showMagnetic, panel_echo_.layers.magnetic, layers_.magnetic);
    DynamicsOptions dynamics = sim_.dynamics();
    pull_toggle(vm.collisions, panel_echo_.collisions, dynamics.collisions);
    pull_toggle(vm.magnetic, panel_echo_.magnetic, dynamics.magnetic);
    sim_.set_dynamics(dynamics);

    FieldParams params = sim_.params();
    if (pull && vm.timeFrac.get() != panel_echo_.time_frac) {
        set_time_scale(std::exp2(kTimeScaleLog2Range.from_fraction(vm.timeFrac.get())));
    }
    if (pull && vm.kFrac.get() != panel_echo_.k_frac) {
        params.k = kCoulombRange.from_fraction(vm.kFrac.get());
    }
    if (pull && vm.epsFrac.get() != panel_echo_.eps_frac) {
        params.softening = kSofteningRange.from_fraction(vm.epsFrac.get());
    }
    if (pull && vm.bFrac.get() != panel_echo_.b_frac) {
        params.b_external = kExternalBRange.from_fraction(vm.bFrac.get());
    }
    if (pull && vm.cFrac.get() != panel_echo_.c_frac) {
        params.light_speed = std::pow(10.f, kLightSpeedLog10Range.from_fraction(vm.cFrac.get()));
    }
    sim_.set_params(params);
    if (pull && vm.newChargeFrac.get() != panel_echo_.new_charge_frac) {
        new_charge_ = kNewChargeRange.from_fraction(vm.newChargeFrac.get());
    }

    // Push state back.
    vm.showPotential = layers_.potential;
    vm.showLines = layers_.lines;
    vm.showGrid = layers_.grid;
    vm.showFlow = layers_.flow;
    vm.showProbe = layers_.probe;
    vm.showTrails = layers_.trails;
    vm.showMagnetic = layers_.magnetic;
    vm.collisions = dynamics.collisions;
    vm.magnetic = dynamics.magnetic;
    panel_echo_.layers = layers_;
    panel_echo_.collisions = dynamics.collisions;
    panel_echo_.magnetic = dynamics.magnetic;

    const float time_frac = kTimeScaleLog2Range.to_fraction(std::log2(time_scale_));
    vm.timeFrac = time_frac;
    panel_echo_.time_frac = time_frac;
    vm.timeFill = percent(time_frac);
    const float k_frac = kCoulombRange.to_fraction(params.k);
    const float eps_frac = kSofteningRange.to_fraction(params.softening);
    const float q_frac = kNewChargeRange.to_fraction(new_charge_);
    vm.kFrac = k_frac;
    vm.epsFrac = eps_frac;
    vm.newChargeFrac = q_frac;
    panel_echo_.k_frac = k_frac;
    panel_echo_.eps_frac = eps_frac;
    panel_echo_.new_charge_frac = q_frac;
    vm.kText = std::format("{:.2f}", params.k);
    vm.kFill = percent(k_frac);
    vm.epsText = std::format("{:.2f}", params.softening);
    vm.epsFill = percent(eps_frac);
    vm.newChargeText = std::format("{:.2f}", new_charge_);
    vm.newChargeFill = percent(q_frac);
    const float b_frac = kExternalBRange.to_fraction(params.b_external);
    vm.bFrac = b_frac;
    panel_echo_.b_frac = b_frac;
    vm.bText = std::format("{:+.2f}", params.b_external);
    vm.bFill = percent(b_frac);
    const float c_frac = kLightSpeedLog10Range.to_fraction(std::log10(params.light_speed));
    vm.cFrac = c_frac;
    panel_echo_.c_frac = c_frac;
    vm.cText = std::format("{:.0f}", params.light_speed);
    vm.cFill = percent(c_frac);

    const engine::loc::Catalog& catalog = world_.ctx<engine::loc::Catalog>();
    vm.statusText = std::format("{} · {}",
            catalog_text(catalog, paused_ ? "status.paused" : "status.running"), time_scale_text(time_scale_));
    vm.statusColor = paused_ ? "#fcd34d" : "#a5f3c4";
    if (scene_cleared_) {
        vm.sceneText = catalog_text(catalog, "status.scene_empty");
    } else {
        const std::string name = catalog_text(catalog, preset_key(preset_));
        const engine::loc::Arg name_arg{"name", std::string_view{name}};
        vm.sceneText = catalog.text("status.scene", std::span<const engine::loc::Arg>(&name_arg, 1)).text;
    }
    vm.pauseLabel = catalog_text(catalog, paused_ ? "panel.resume" : "panel.pause");
    paint_panel_locale();
    vm.timeScaleText = time_scale_text(time_scale_);
    vm.panelRight = std::format("{}", panel_visible_ ? 12.f : kPanelHiddenRight);

    const auto& charges = sim_.charges();
    const float kinetic = kinetic_energy(charges);
    const float potential = potential_energy(charges, params);
    vm.energyKinetic = std::format("{:.3f}", kinetic);
    vm.energyPotential = std::format("{:.3f}", potential);
    vm.energyTotal = std::format("{:.3f}", kinetic + potential);
    const auto fixed_count = std::count_if(charges.begin(), charges.end(), [](const Charge& c) { return c.fixed; });
    vm.chargeCount = std::format("{} / {}", static_cast<std::ptrdiff_t>(charges.size()) - fixed_count, fixed_count);
    float fastest = 0.f;
    for (const Charge& c : charges) {
        if (!c.fixed) {
            fastest = std::max(fastest, glm::length(c.velocity));
        }
    }
    vm.speedRatio = std::format("{:.2f}", params.light_speed > 0.f ? fastest / params.light_speed : 0.f);

    // Probe readout next to the cursor; flipped to the other side near the window edges.
    const bool show_probe =
            layers_.probe && !hovered_ && !drag_ && !pan_grab_ && !pointer_over_ui() && !help_->is_open();
    vm.probeVisibility = show_probe ? "visible" : "hidden";
    if (show_probe) {
        const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
        float x = pointer_screen_.x + kProbeOffsetPx;
        float y = pointer_screen_.y + kProbeOffsetPx;
        if (x + kProbeBoxW > static_cast<float>(window.width)) {
            x = pointer_screen_.x - kProbeOffsetPx - kProbeBoxW;
        }
        const float probe_box_h = layers_.magnetic ? kProbeBoxHMagnetic : kProbeBoxH;
        if (y + probe_box_h > static_cast<float>(window.height)) {
            y = pointer_screen_.y - kProbeOffsetPx - probe_box_h;
        }
        vm.probeLeft = std::format("{:.0f}", x);
        vm.probeTop = std::format("{:.0f}", y);
        const glm::vec3 e = field_at(charges, pointer_world_, params);
        // TeX for the <Math> readouts; ASCII '-' is typeset as a real minus.
        vm.probeField = std::format("|E| = F/q = {:.3f}", glm::length(e));
        vm.probeComponents = std::format("E = ({:.3f},\\, {:.3f})", e.x, e.y);
        vm.probePotential = std::format("\\varphi = {:.3f}", potential_at(charges, pointer_world_, params));
        vm.probeMagneticDisplay = layers_.magnetic ? "block" : "none";
        if (layers_.magnetic) {
            vm.probeMagnetic = std::format("B_z = {:.3f}", magnetic_z(charges, pointer_world_, params, kNoCharge, sim_.coils()));
        }
    } else {
        vm.probeMagneticDisplay = "none";
    }
}

}
