#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/resources/assets_db.h>

#include <game/camera_control.h>
#include <game/charge.h>
#include <game/electrostatics.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace game {

// Uniform names must match charge.shader and potential.shader.
class ChargeView {
public:
    ChargeView(engine::ecs::World& world, engine::AssetsDb& assets);

    struct Frame {
        std::span<const Charge> charges;
        FieldParams params;
        CameraView camera;
        bool show_potential = true;
        std::optional<std::size_t> hovered;
    };

    void update(const Frame& frame);

private:
    void sync_charges(const Frame& frame);
    void sync_potential(const Frame& frame);

    engine::ecs::World& world_;
    engine::AssetsDb& assets_;
    engine::ecs::Entity potential_layer_{};
    std::vector<engine::ecs::Entity> charge_views_;
};

}
