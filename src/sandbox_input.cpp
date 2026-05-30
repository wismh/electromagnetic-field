#include <game/sandbox_input.h>

#include <game/charge_limit.h>

#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>
#include <engine/log.h>
#include <engine/ui/canvas.h>

#include <format>

namespace game {

SandboxInput::SandboxInput(engine::ecs::World& world, engine::ecs::Entity camera, Simulation& sim, FieldLayers& layers,
        SimClock& clock, float& new_charge, bool& panel_visible, InputActions actions, SandboxHooks hooks) :
    world_(world),
    camera_(camera),
    sim_(sim),
    layers_(layers),
    clock_(clock),
    new_charge_(new_charge),
    panel_visible_(panel_visible),
    actions_(std::move(actions)),
    hooks_(std::move(hooks)) {}

void SandboxInput::handle_mouse(bool overlays_open) {
    for (const engine::MouseEvent& event : engine::ecs::EventReader<engine::MouseEvent>{world_, mouse_cursor_}) {
        if (event.window != engine::kPrimaryWindow) {
            continue;
        }
        // Up never sets MouseConsumed, so a release-only frame must not clear the latch.
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
                if (pointer_blocked(overlays_open)) {
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
                if (pointer_blocked(overlays_open)) {
                    break;
                }
                pointer_world_ = pointer_to_world(event.position);
                update_hover();
                on_wheel(event.wheel_y);
                break;
        }
    }
}

void SandboxInput::on_action(engine::ActionId action) {
    if (const std::optional<Preset> preset = actions_.preset_for(action)) {
        hooks_.load_preset(*preset);
        return;
    }
    if (action == actions_.pause) {
        clock_.paused = !clock_.paused;
        return;
    }
    if (action == actions_.panel) {
        panel_visible_ = !panel_visible_;
        return;
    }
    if (action == actions_.up) {
        clock_.set_scale(clock_.scale * 2.f);
        return;
    }
    if (action == actions_.down) {
        clock_.set_scale(clock_.scale * 0.5f);
        return;
    }
    if (action == actions_.potential) {
        layers_.potential = !layers_.potential;
        return;
    }
    if (action == actions_.lines) {
        layers_.lines = !layers_.lines;
        return;
    }
    if (action == actions_.grid) {
        layers_.grid = !layers_.grid;
        return;
    }
    if (action == actions_.flow) {
        layers_.flow = !layers_.flow;
        return;
    }
    if (action == actions_.probe) {
        layers_.probe = !layers_.probe;
        return;
    }
    if (action == actions_.trails) {
        layers_.trails = !layers_.trails;
        return;
    }
    if (action == actions_.magnetic_layer) {
        layers_.magnetic = !layers_.magnetic;
        return;
    }
    if (action == actions_.collisions) {
        DynamicsOptions dynamics = sim_.dynamics();
        dynamics.collisions = !dynamics.collisions;
        sim_.set_dynamics(dynamics);
        return;
    }
    if (action == actions_.magnetic) {
        DynamicsOptions dynamics = sim_.dynamics();
        dynamics.magnetic = !dynamics.magnetic;
        sim_.set_dynamics(dynamics);
        return;
    }
    if (action == actions_.reset_camera) {
        set_camera_view(CameraView{.position = {0.f, 0.f, 0.f}, .ortho_half = kDefaultOrthoHalf});
        return;
    }
    if (action == actions_.clear) {
        hooks_.clear();
        return;
    }
    if (action == actions_.reload) {
        hooks_.reload();
        return;
    }
    if (!hovered_) {
        return;
    }
    Charge& c = sim_.charges()[*hovered_];
    if (action == actions_.fix_charge) {
        if (drag_ && drag_->index == *hovered_) {
            drag_->was_fixed = !drag_->was_fixed;
        } else {
            c.fixed = !c.fixed;
            c.velocity = glm::vec3{0.f};
        }
        return;
    }
    if (action == actions_.flip_charge) {
        c.q = -c.q;
        return;
    }
    if (action == actions_.remove_charge) {
        remove_charge(*hovered_);
    }
}

void SandboxInput::step() {
    if (clock_.paused) {
        clock_.step_requested = true;
    }
}

void SandboxInput::refresh_pointer() {
    pointer_world_ = pointer_to_world(pointer_screen_);
    on_mouse_move();
    update_hover();
}

void SandboxInput::end_drag() {
    if (!drag_) {
        return;
    }
    Charge& c = sim_.charges()[drag_->index];
    c.fixed = drag_->was_fixed;
    c.velocity = glm::vec3{0.f};
    drag_.reset();
}

void SandboxInput::clear_hover() {
    hovered_.reset();
}

void SandboxInput::assume_pointer_on_ui() {
    pointer_on_ui_ = true;
}

bool SandboxInput::pointer_on_ui() const {
    return pointer_on_ui_;
}

bool SandboxInput::dragging() const {
    return drag_.has_value();
}

bool SandboxInput::panning() const {
    return pan_grab_.has_value();
}

bool SandboxInput::hovering_charge() const {
    return hovered_.has_value();
}

glm::vec2 SandboxInput::pointer_screen() const {
    return pointer_screen_;
}

const glm::vec3& SandboxInput::pointer_world() const {
    return pointer_world_;
}

std::optional<std::size_t> SandboxInput::hovered() const {
    return hovered_;
}

// LMB grabs or places +q. RMB deletes or places -q. MMB pans.
void SandboxInput::on_mouse_down(engine::MouseButton button) {
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
            // Fixed while held, so the integrator does not fight the cursor.
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

void SandboxInput::on_mouse_up(engine::MouseButton button) {
    if (button == engine::MouseButton::Left) {
        end_drag();
    } else if (button == engine::MouseButton::Middle) {
        pan_grab_.reset();
    }
}

void SandboxInput::on_mouse_move() {
    if (!drag_) {
        return;
    }
    Charge& c = sim_.charges()[drag_->index];
    c.position = pointer_world_ + drag_->offset;
    c.position.z = 0.f;
}

// Over a charge the wheel changes |q|; elsewhere it zooms about the cursor.
void SandboxInput::on_wheel(float notches) {
    if (hovered_) {
        Charge& c = sim_.charges()[*hovered_];
        c.q = step_charge_magnitude(c.q, notches);
        return;
    }
    set_camera_view(zoom_about(camera_view(), pointer_world_, notches));
}

void SandboxInput::add_charge(float q) {
    if (sim_.charges().size() >= kMaxCharges) {
        engine::log::warn(std::format("charge limit reached ({})", kMaxCharges));
        return;
    }
    sim_.charges().push_back(Charge{.position = {pointer_world_.x, pointer_world_.y, 0.f}, .q = q});
}

void SandboxInput::remove_charge(std::size_t index) {
    end_drag();
    auto& charges = sim_.charges();
    charges.erase(charges.begin() + static_cast<std::ptrdiff_t>(index));
    hovered_.reset();
}

void SandboxInput::update_hover() {
    if (drag_) {
        hovered_ = drag_->index;
        return;
    }
    hovered_ = pick_charge(sim_.charges(), pointer_world_);
}

glm::vec3 SandboxInput::pointer_to_world(glm::vec2 screen) const {
    const engine::ui::WindowSize window = engine::ui::window_size_for(world_, engine::kPrimaryWindow);
    if (window.width <= 0 || window.height <= 0) {
        return pointer_world_;
    }
    glm::vec3 p = engine::screen_to_world(
            screen, world_.get<engine::Camera>(camera_), world_.get<engine::Transform>(camera_), window);
    p.z = 0.f;
    return p;
}

CameraView SandboxInput::camera_view() const {
    return CameraView{
            .position = world_.get<engine::Transform>(camera_).position,
            .ortho_half = world_.get<engine::Camera>(camera_).ortho_size,
    };
}

void SandboxInput::set_camera_view(const CameraView& view) {
    world_.get<engine::Transform>(camera_).position = view.position;
    world_.get<engine::Camera>(camera_).ortho_size = view.ortho_half;
}

bool SandboxInput::pointer_blocked(bool overlays_open) const {
    return pointer_on_ui_ || overlays_open;
}

}
