#include <game/game.h>

#include <engine/builtin_ids.h>
#include <engine/core/time.h>
#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>
#include <engine/log.h>
#include <engine/render/renderable.h>
#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

namespace game {
namespace {

constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
constexpr float kOrthoHalf = 10.f;
// Large enough to cover the view at any window aspect.
constexpr float kPotentialLayerSize = 200.f;
// Must match kQuadRadii in assets/shaders/charge.shader.
constexpr float kChargeQuadRadii = 3.f;

constexpr float kMinTimeScale = 0.125f;
constexpr float kMaxTimeScale = 8.f;

constexpr glm::vec4 kPositiveColor{1.f, 0.42f, 0.2f, 1.f};
constexpr glm::vec4 kNegativeColor{0.25f, 0.55f, 1.f, 1.f};

}

Game::Game(engine::AssetsDb& assets) :
    assets_(assets) {}

engine::WindowDesc Game::primary_window() const {
    return {.title = "Electromagnetic Field", .size = {kWindowW, kWindowH}};
}

void Game::on_start() {
    spawn_camera();
    spawn_potential_layer();
    load_preset(preset_);
}

void Game::on_fixed_update() {
    if (!paused_ || step_requested_) {
        // Sub-step so a high time scale does not mean a large, unstable dt.
        const float scale = step_requested_ ? 1.f : time_scale_;
        const int substeps = std::max(1, static_cast<int>(std::ceil(scale)));
        const float dt = engine::kFixed * scale / static_cast<float>(substeps);
        for (int i = 0; i < substeps; ++i) {
            sim_.step(dt);
        }
        step_requested_ = false;
    }
    GameBase::on_fixed_update();
}

void Game::on_update() {
    handle_mouse();
    handle_keys();
    update_hover();
    sync_charge_views();
    sync_potential_uniforms();
    GameBase::on_update();
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
            .scale = {kPotentialLayerSize, kPotentialLayerSize, 1.f},
    });
    world_.emplace<engine::render::Renderable>(potential_layer_, engine::render::Renderable{
            .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
            .material = assets_.get<engine::render::IMaterial>(assets::materials::potential),
            .layer = -100,
            .material_override = engine::render::MaterialOverride{},
    });
}

void Game::handle_mouse() {
    for (const engine::MouseEvent& event : engine::ecs::EventReader<engine::MouseEvent>{world_, mouse_cursor_}) {
        if (event.window != engine::kPrimaryWindow) {
            continue;
        }
        switch (event.kind) {
            case engine::MouseEvent::Kind::Move:
                pointer_world_ = pointer_to_world(event.position);
                on_mouse_move();
                break;
            case engine::MouseEvent::Kind::Down:
                pointer_world_ = pointer_to_world(event.position);
                update_hover();
                on_mouse_down(event.button);
                break;
            case engine::MouseEvent::Kind::Up:
                pointer_world_ = pointer_to_world(event.position);
                on_mouse_up(event.button);
                break;
            case engine::MouseEvent::Kind::Wheel:
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

// LMB: grab a charge or place +1. RMB: delete a charge or place -1.
void Game::on_mouse_down(engine::MouseButton button) {
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
            add_charge(1.f);
        }
    } else if (button == engine::MouseButton::Right) {
        if (hovered_) {
            remove_charge(*hovered_);
        } else {
            add_charge(-1.f);
        }
    }
}

void Game::on_mouse_up(engine::MouseButton button) {
    if (button == engine::MouseButton::Left) {
        end_drag();
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

void Game::on_wheel(float notches) {
    if (!hovered_) {
        return;
    }
    Charge& c = sim_.charges()[*hovered_];
    c.q = step_charge_magnitude(c.q, notches);
}

void Game::on_key(engine::KeyCode key) {
    using engine::KeyCode;
    switch (key) {
        case KeyCode::Space:
            paused_ = !paused_;
            engine::log::info(paused_ ? "paused" : "running");
            return;
        case KeyCode::Right:
            if (paused_) {
                step_requested_ = true;
            }
            return;
        case KeyCode::Up:
        case KeyCode::Down:
            time_scale_ = std::clamp(key == KeyCode::Up ? time_scale_ * 2.f : time_scale_ * 0.5f, kMinTimeScale,
                    kMaxTimeScale);
            engine::log::info(std::format("time scale x{}", time_scale_));
            return;
        case KeyCode::C:
            end_drag();
            sim_.charges().clear();
            hovered_.reset();
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
    sim_.charges() = make_preset(preset);
    hovered_.reset();
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

}
