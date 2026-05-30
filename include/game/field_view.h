#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/render/particles.h>
#include <engine/resources/assets_db.h>
#include <engine/resources/asset_id.h>

#include <game/charge.h>
#include <game/electrostatics.h>
#include <game/field_layers.h>
#include <game/field_viz.h>
#include <game/magnetism.h>
#include <game/trails.h>

#include <glm/vec3.hpp>

#include <optional>
#include <span>
#include <vector>

namespace game {

// Emitter is never stepped; particles are rebuilt every frame as an instanced draw list.
class FieldView {
public:
    FieldView(engine::ecs::World& world, engine::AssetsDb& assets);

    struct Frame {
        std::span<const Charge> charges;
        std::span<const Coil> coils;
        FieldParams params;
        Bounds view;
        float world_per_pixel = 0.f;
        float dt = 0.f;  // real time; flow tracers keep moving while paused
        std::optional<glm::vec3> probe;
        const Trails* trails = nullptr;
        float sim_time = 0.f;
        FieldLayers layers;
        // Off: Bz marks are drawn faded.
        bool magnetic_force = false;
    };

    void update(const Frame& frame);

private:
    engine::ecs::Entity spawn_layer(engine::AssetId material, int layer);
    engine::render::ParticleEmitter& emitter(engine::ecs::Entity entity);

    void build_lines(const Frame& frame);
    void build_grid(const Frame& frame);
    void build_magnetic(const Frame& frame);
    void build_coils(const Frame& frame);
    void build_flow(const Frame& frame);
    void build_probe(const Frame& frame);
    void build_trails(const Frame& frame);

    [[nodiscard]] bool lines_dirty(const Frame& frame) const;

    engine::ecs::World& world_;
    engine::AssetsDb& assets_;
    engine::ecs::Entity line_segments_{};
    engine::ecs::Entity line_heads_{};
    engine::ecs::Entity grid_arrows_{};
    engine::ecs::Entity magnetic_marks_{};
    engine::ecs::Entity coil_segments_{};
    engine::ecs::Entity coil_heads_{};
    engine::ecs::Entity flow_dots_{};
    engine::ecs::Entity probe_arrow_{};
    engine::ecs::Entity trail_segments_{};

    FlowField flow_;

    std::vector<Charge> traced_charges_;
    Bounds traced_view_{};
    float traced_world_per_pixel_ = 0.f;
    std::vector<engine::render::Particle> line_segment_cache_;
    std::vector<engine::render::Particle> line_head_cache_;
};

}
