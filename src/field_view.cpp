#include <game/field_view.h>

#include <asset_ids.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace game {
namespace {

// Draw order: potential background is -100, charges are 0.
constexpr int kGridLayer = -60;
constexpr int kLinesLayer = -50;
constexpr int kFlowLayer = -40;
constexpr int kProbeLayer = 10;

constexpr float kLineWidthPx = 2.2f;
constexpr float kHeadSpacingPx = 140.f;
constexpr float kHeadLengthPx = 11.f;
constexpr float kHeadWidthPx = 9.f;
constexpr float kGridSpacingPx = 46.f;
constexpr float kFlowWidthPx = 4.f;
// Streak length = distance travelled in this much time (a motion-blur trail).
constexpr float kFlowTrailSeconds = 0.3f;
constexpr float kProbeLengthPx = 90.f;
constexpr float kProbeWidthPx = 16.f;
// Field lines are traced a bit past the screen edge so they do not visibly stop at it.
constexpr float kLineBoundsMargin = 0.5f;

// Lives long enough that update_emitter never ages it out; start == end keeps size/colour constant.
engine::render::Particle make_static(glm::vec3 position, float rotation, glm::vec2 size, glm::vec4 color) {
    engine::render::Particle p;
    p.position = position;
    p.rotation = rotation;
    p.size = size;
    p.base_size = size;
    p.start_size = size;
    p.end_size = size;
    p.color = color;
    p.start_color = color;
    p.end_color = color;
    p.lifetime = 1e9f;
    return p;
}

Bounds expand(const Bounds& b, float fraction) {
    const glm::vec2 margin = (b.max - b.min) * fraction;
    return Bounds{b.min - margin, b.max + margin};
}

// Perceptual "magma"-like ramp for |E| on the vector grid.
glm::vec3 magma(float t) {
    constexpr std::array<glm::vec3, 5> kStops{
            glm::vec3{0.23f, 0.06f, 0.43f},
            glm::vec3{0.55f, 0.16f, 0.51f},
            glm::vec3{0.87f, 0.31f, 0.39f},
            glm::vec3{0.99f, 0.62f, 0.36f},
            glm::vec3{0.99f, 0.95f, 0.75f},
    };
    const float x = std::clamp(t, 0.f, 1.f) * static_cast<float>(kStops.size() - 1);
    const auto i = std::min(static_cast<std::size_t>(x), kStops.size() - 2);
    return glm::mix(kStops[i], kStops[i + 1], x - static_cast<float>(i));
}

glm::vec4 line_color(float strength) {
    const glm::vec3 dim{0.45f, 0.5f, 0.62f};
    const glm::vec3 bright{0.95f, 0.97f, 1.f};
    return {glm::mix(dim, bright, strength), 0.35f + 0.5f * strength};
}

bool same_charges(std::span<const Charge> a, std::span<const Charge> b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const Charge& x, const Charge& y) {
        return x.position == y.position && x.q == y.q;
    });
}

}

FieldView::FieldView(engine::ecs::World& world, engine::AssetsDb& assets) :
    world_(world),
    assets_(assets) {
    line_segments_ = spawn_layer(assets::materials::segment, kLinesLayer);
    line_heads_ = spawn_layer(assets::materials::head, kLinesLayer);
    grid_arrows_ = spawn_layer(assets::materials::arrow, kGridLayer);
    flow_dots_ = spawn_layer(assets::materials::dot, kFlowLayer);
    probe_arrow_ = spawn_layer(assets::materials::arrow, kProbeLayer);
    emitter(line_heads_).order_in_layer = 1;
}

engine::ecs::Entity FieldView::spawn_layer(engine::AssetId material, int layer) {
    const engine::ecs::Entity entity = world_.create();
    engine::render::ParticleEmitter em;
    em.playing = false;
    em.material = assets_.get<engine::render::IMaterial>(material);
    em.layer = layer;
    world_.emplace<engine::render::ParticleEmitter>(entity, std::move(em));
    return entity;
}

engine::render::ParticleEmitter& FieldView::emitter(engine::ecs::Entity entity) {
    return world_.get<engine::render::ParticleEmitter>(entity);
}

void FieldView::update(const Frame& frame) {
    build_lines(frame);
    build_grid(frame);
    build_flow(frame);
    build_probe(frame);
}

bool FieldView::lines_dirty(const Frame& frame) const {
    return !same_charges(frame.charges, traced_charges_) || frame.view.min != traced_view_.min ||
            frame.view.max != traced_view_.max || frame.world_per_pixel != traced_world_per_pixel_;
}

void FieldView::build_lines(const Frame& frame) {
    auto& segments = emitter(line_segments_).particles;
    auto& heads = emitter(line_heads_).particles;
    if (!frame.layers.lines) {
        segments.clear();
        heads.clear();
        return;
    }
    if (lines_dirty(frame)) {
        traced_charges_.assign(frame.charges.begin(), frame.charges.end());
        traced_view_ = frame.view;
        traced_world_per_pixel_ = frame.world_per_pixel;
        line_segment_cache_.clear();
        line_head_cache_.clear();

        FieldLineOptions options;
        options.bounds = expand(frame.view, kLineBoundsMargin);
        const float width = kLineWidthPx * frame.world_per_pixel;
        const float head_spacing = kHeadSpacingPx * frame.world_per_pixel;
        const glm::vec2 head_size{kHeadLengthPx * frame.world_per_pixel, kHeadWidthPx * frame.world_per_pixel};

        for (const FieldLine& line : trace_field_lines(frame.charges, frame.params, options)) {
            // Stagger the first head per line so heads on neighbouring lines do not form rings.
            float until_head = 0.5f * head_spacing;
            for (std::size_t i = 0; i + 1 < line.size(); ++i) {
                const glm::vec3 a = line[i];
                const glm::vec3 b = line[i + 1];
                const glm::vec3 d = b - a;
                const float length = glm::length(d);
                if (length <= 0.f) {
                    continue;
                }
                const glm::vec3 mid = 0.5f * (a + b);
                const float angle = std::atan2(d.y, d.x);
                const glm::vec4 color = line_color(field_strength01(glm::length(field_at(frame.charges, mid,
                        frame.params))));
                // Extend each segment by the line width so joints between segments have no gaps.
                line_segment_cache_.push_back(make_static(mid, angle, {length + width, width}, color));

                until_head -= length;
                if (until_head <= 0.f && frame.view.contains(mid)) {
                    line_head_cache_.push_back(make_static(mid, angle, head_size, {color.r, color.g, color.b, 0.9f}));
                    until_head = head_spacing;
                }
            }
        }
    }
    segments = line_segment_cache_;
    heads = line_head_cache_;
}

void FieldView::build_grid(const Frame& frame) {
    auto& arrows = emitter(grid_arrows_).particles;
    arrows.clear();
    if (!frame.layers.grid) {
        return;
    }
    const float spacing = kGridSpacingPx * frame.world_per_pixel;
    for (const FieldSample& s : sample_field_grid(frame.charges, frame.params, frame.view, spacing)) {
        const float magnitude = glm::length(s.field);
        if (magnitude <= 0.f) {
            continue;
        }
        const float strength = field_strength01(magnitude);
        const float length = spacing * (0.3f + 0.55f * strength);
        const float angle = std::atan2(s.field.y, s.field.x);
        arrows.push_back(make_static(s.position, angle, {length, 0.3f * spacing},
                glm::vec4{magma(strength), 0.55f + 0.45f * strength}));
    }
}

void FieldView::build_flow(const Frame& frame) {
    auto& dots = emitter(flow_dots_).particles;
    dots.clear();
    if (!frame.layers.flow) {
        return;
    }
    const Bounds bounds = expand(frame.view, 0.05f);
    flow_.update(frame.charges, frame.params, bounds, frame.dt);

    const float width = kFlowWidthPx * frame.world_per_pixel;
    for (const FlowParticle& p : flow_.particles()) {
        const float speed = glm::length(p.velocity);
        const float fade_in = glm::smoothstep(0.f, 0.4f, p.age);
        const float fade_out = 1.f - glm::smoothstep(p.lifetime - 0.6f, p.lifetime, p.age);
        const float alpha = fade_in * fade_out;
        if (alpha <= 0.01f) {
            continue;
        }
        const float length = std::max(width, kFlowTrailSeconds * speed);
        const glm::vec3 dir = speed > 0.f ? p.velocity / speed : glm::vec3{1.f, 0.f, 0.f};
        // Centre the streak behind the particle so its bright head leads.
        dots.push_back(make_static(p.position - 0.5f * length * dir, std::atan2(dir.y, dir.x), {length, width},
                {0.6f, 0.88f, 1.f, alpha}));
    }
}

void FieldView::build_probe(const Frame& frame) {
    auto& arrow = emitter(probe_arrow_).particles;
    arrow.clear();
    if (!frame.layers.probe || !frame.probe) {
        return;
    }
    // A unit positive test charge: the arrow shows E, which equals F = qE for q = +1.
    const glm::vec3 e = field_at(frame.charges, *frame.probe, frame.params);
    const float magnitude = glm::length(e);
    if (magnitude <= 0.f) {
        return;
    }
    const float strength = field_strength01(magnitude);
    const float length = kProbeLengthPx * frame.world_per_pixel * (0.35f + 0.65f * strength);
    const glm::vec3 dir = e / magnitude;
    // The arrow quad is centred on its position, so shift it to start at the cursor.
    arrow.push_back(make_static(*frame.probe + 0.5f * length * dir, std::atan2(dir.y, dir.x),
            {length, kProbeWidthPx * frame.world_per_pixel}, {1.f, 1.f, 1.f, 0.95f}));
}

}
