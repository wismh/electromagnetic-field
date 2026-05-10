#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/render/particles.h>
#include <engine/resources/assets_db.h>
#include <engine/resources/asset_id.h>

#include <game/charge.h>
#include <game/electrostatics.h>
#include <game/field_viz.h>

#include <glm/vec3.hpp>

#include <optional>
#include <span>
#include <vector>

namespace game {

struct FieldLayers {
    bool potential = true;
    bool lines = true;
    bool grid = false;
    bool flow = true;
    bool probe = true;
};

// Draws field lines, the vector grid, flow tracers and the cursor probe. Every element is an
// instance in a ParticleEmitter that never emits: the emitter is only used as an instanced,
// layer-sorted draw list whose particles are rebuilt here every frame.
class FieldView {
public:
    FieldView(engine::ecs::World& world, engine::AssetsDb& assets);

    struct Frame {
        std::span<const Charge> charges;
        FieldParams params;
        Bounds view;              // visible world rectangle
        float world_per_pixel = 0.f;
        float dt = 0.f;           // simulation time advanced this frame (0 while paused)
        std::optional<glm::vec3> probe;
        FieldLayers layers;
    };

    void update(const Frame& frame);

private:
    engine::ecs::Entity spawn_layer(engine::AssetId material, int layer);
    engine::render::ParticleEmitter& emitter(engine::ecs::Entity entity);

    void build_lines(const Frame& frame);
    void build_grid(const Frame& frame);
    void build_flow(const Frame& frame);
    void build_probe(const Frame& frame);

    [[nodiscard]] bool lines_dirty(const Frame& frame) const;

    engine::ecs::World& world_;
    engine::AssetsDb& assets_;
    engine::ecs::Entity line_segments_{};
    engine::ecs::Entity line_heads_{};
    engine::ecs::Entity grid_arrows_{};
    engine::ecs::Entity flow_dots_{};
    engine::ecs::Entity probe_arrow_{};

    FlowField flow_;

    // Field lines are re-traced only when the charges or the view change.
    std::vector<Charge> traced_charges_;
    Bounds traced_view_{};
    float traced_world_per_pixel_ = 0.f;
    std::vector<engine::render::Particle> line_segment_cache_;
    std::vector<engine::render::Particle> line_head_cache_;
};

}
