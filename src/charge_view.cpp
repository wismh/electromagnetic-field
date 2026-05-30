#include <game/charge_view.h>

#include <game/charge_limit.h>

#include <engine/builtin_ids.h>
#include <engine/ecs/transform.h>
#include <engine/render/renderable.h>

#include <asset_ids.h>

#include <glm/vec4.hpp>

#include <algorithm>
#include <string>

namespace game {
namespace {

constexpr float kPotentialLayerViewHeights = 4.f;
constexpr float kChargeQuadRadii = 3.f;  // must match kQuadRadii in charge.shader

constexpr glm::vec4 kPositiveColor{1.f, 0.42f, 0.2f, 1.f};
constexpr glm::vec4 kNegativeColor{0.25f, 0.55f, 1.f, 1.f};

}

ChargeView::ChargeView(engine::ecs::World& world, engine::AssetsDb& assets) :
    world_(world),
    assets_(assets) {
    potential_layer_ = world_.create();
    world_.emplace<engine::Transform>(potential_layer_, engine::Transform{});
    world_.emplace<engine::render::Renderable>(potential_layer_, engine::render::Renderable{
            .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
            .material = assets_.get<engine::render::IMaterial>(assets::materials::potential),
            .layer = -100,
            .material_override = engine::render::MaterialOverride{},
    });
}

void ChargeView::update(const Frame& frame) {
    // No visibility flag: hide by collapsing the quad.
    const float size = frame.show_potential ? 2.f * frame.camera.ortho_half * kPotentialLayerViewHeights : 0.f;
    auto& background = world_.get<engine::Transform>(potential_layer_);
    background.position = {frame.camera.position.x, frame.camera.position.y, 0.f};
    background.scale = {size, size, 1.f};
    sync_charges(frame);
    sync_potential(frame);
}

void ChargeView::sync_charges(const Frame& frame) {
    while (charge_views_.size() < frame.charges.size()) {
        const engine::ecs::Entity view = world_.create();
        world_.emplace<engine::Transform>(view);
        world_.emplace<engine::render::Renderable>(view, engine::render::Renderable{
                .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
                .material = assets_.get<engine::render::IMaterial>(assets::materials::charge),
                .material_override = engine::render::MaterialOverride{},
        });
        charge_views_.push_back(view);
    }
    while (charge_views_.size() > frame.charges.size()) {
        world_.destroy(charge_views_.back());
        charge_views_.pop_back();
    }

    for (std::size_t i = 0; i < frame.charges.size(); ++i) {
        const Charge& c = frame.charges[i];
        const float size = 2.f * kChargeQuadRadii * charge_radius(c.q);
        auto& transform = world_.get<engine::Transform>(charge_views_[i]);
        transform.position = c.position;
        transform.scale = {size, size, 1.f};

        auto& renderable = world_.get<engine::render::Renderable>(charge_views_[i]);
        renderable.color = c.q >= 0.f ? kPositiveColor : kNegativeColor;
        renderable.order_in_layer = frame.hovered == i ? 1 : 0;
        renderable.material_override->set_vec4("uStyle", {
                c.q >= 0.f ? 1.f : -1.f,
                c.fixed ? 1.f : 0.f,
                frame.hovered == i ? 1.f : 0.f,
                0.f,
        });
    }
}

void ChargeView::sync_potential(const Frame& frame) {
    auto& renderable = world_.get<engine::render::Renderable>(potential_layer_);
    engine::render::MaterialOverride& override = *renderable.material_override;
    const std::size_t count = std::min(frame.charges.size(), kMaxCharges);
    for (std::size_t i = 0; i < count; ++i) {
        const Charge& c = frame.charges[i];
        override.set_vec4("uCharges[" + std::to_string(i) + "]", {c.position.x, c.position.y, c.q, 0.f});
    }
    override.set_vec4("uParams", {static_cast<float>(count), frame.params.k, frame.params.softening, 0.f});
}

}
