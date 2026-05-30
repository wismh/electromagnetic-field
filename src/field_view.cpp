#include <game/field_view.h>

#include <game/magnetism.h>

#include <asset_ids.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace game {
namespace {

constexpr int kMagneticLayer = -70;  // potential is -100, charges are 0
constexpr float kInactiveMagneticAlpha = 0.35f;
constexpr int kGridLayer = -60;
constexpr int kLinesLayer = -50;
constexpr int kFlowLayer = -40;
constexpr int kTrailLayer = -30;
constexpr int kCoilLayer = -20;  // above the field layers, below the charges
constexpr float kCoilWidthPx = 5.f;
constexpr int kCoilDrawSegments = 96;
constexpr int kCoilArrows = 8;
constexpr glm::vec4 kCoilColor{0.93f, 0.62f, 0.32f, 0.95f};
constexpr int kProbeLayer = 10;

constexpr float kLineWidthPx = 2.2f;
constexpr float kHeadSpacingPx = 140.f;
constexpr float kHeadLengthPx = 11.f;
constexpr float kHeadWidthPx = 9.f;
constexpr float kGridSpacingPx = 46.f;
constexpr float kFlowWidthPx = 4.f;
constexpr float kFlowTrailSeconds = 0.3f;
constexpr float kTrailWidthPx = 3.f;
constexpr float kProbeLengthPx = 90.f;
constexpr float kProbeWidthPx = 16.f;
constexpr float kLineBoundsMargin = 0.5f;

// Lifetime long enough that update_emitter never ages it out. start == end keeps size and colour fixed.
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
    magnetic_marks_ = spawn_layer(assets::materials::bmark, kMagneticLayer);
    flow_dots_ = spawn_layer(assets::materials::dot, kFlowLayer);
    probe_arrow_ = spawn_layer(assets::materials::arrow, kProbeLayer);
    trail_segments_ = spawn_layer(assets::materials::segment, kTrailLayer);
    coil_segments_ = spawn_layer(assets::materials::segment, kCoilLayer);
    coil_heads_ = spawn_layer(assets::materials::head, kCoilLayer);
    emitter(coil_heads_).order_in_layer = 1;
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
    build_magnetic(frame);
    build_coils(frame);
    build_flow(frame);
    build_probe(frame);
    build_trails(frame);
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

void FieldView::build_magnetic(const Frame& frame) {
    auto& marks = emitter(magnetic_marks_).particles;
    marks.clear();
    if (!frame.layers.magnetic) {
        return;
    }
    const float spacing = kGridSpacingPx * frame.world_per_pixel;
    if (spacing <= 0.f) {
        return;
    }
    const float half = 0.5f * spacing;  // off the vector grid, so marks do not sit on the arrows
    const float x0 = std::ceil((frame.view.min.x - half) / spacing) * spacing + half;
    const float y0 = std::ceil((frame.view.min.y - half) / spacing) * spacing + half;
    for (float y = y0; y <= frame.view.max.y; y += spacing) {
        for (float x = x0; x <= frame.view.max.x; x += spacing) {
            const glm::vec3 p{x, y, 0.f};
            if (inside_charge_glyph(frame.charges, p)) {
                continue;
            }
            const float field_z = magnetic_z(frame.charges, p, frame.params, kNoCharge, frame.coils);
            const float magnitude = std::abs(field_z);
            if (magnitude <= 1e-8f) {
                continue;
            }
            const float strength = field_strength01(magnitude);
            const float side = spacing * (0.22f + 0.28f * strength);
            // r > b is a dot (out of the page); b > r is a cross. The shader branches on that.
            const float alpha = (0.55f + 0.4f * strength) * (frame.magnetic_force ? 1.f : kInactiveMagneticAlpha);
            const glm::vec4 color = field_z > 0.f ? glm::vec4{0.98f, 0.72f, 0.35f, alpha}
                                                   : glm::vec4{0.35f, 0.72f, 0.98f, alpha};
            marks.push_back(make_static(p, 0.f, {side, side}, color));
        }
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
    const glm::vec3 e = field_at(frame.charges, *frame.probe, frame.params);
    const float magnitude = glm::length(e);
    if (magnitude <= 0.f) {
        return;
    }
    const float strength = field_strength01(magnitude);
    const float length = kProbeLengthPx * frame.world_per_pixel * (0.35f + 0.65f * strength);
    const glm::vec3 dir = e / magnitude;
    arrow.push_back(make_static(*frame.probe + 0.5f * length * dir, std::atan2(dir.y, dir.x),
            {length, kProbeWidthPx * frame.world_per_pixel}, {1.f, 1.f, 1.f, 0.95f}));
}

void FieldView::build_trails(const Frame& frame) {
    auto& segments = emitter(trail_segments_).particles;
    segments.clear();
    if (!frame.layers.trails || frame.trails == nullptr) {
        return;
    }
    const float width = kTrailWidthPx * frame.world_per_pixel;
    const float duration = std::max(frame.trails->duration, 1e-3f);
    for (const Trail& trail : frame.trails->trails()) {
        const glm::vec3 tint = trail.q >= 0.f ? glm::vec3{1.f, 0.85f, 0.65f} : glm::vec3{0.65f, 0.85f, 1.f};
        for (std::size_t i = 0; i + 1 < trail.points.size(); ++i) {
            const TrailPoint& a = trail.points[i];
            const TrailPoint& b = trail.points[i + 1];
            const glm::vec3 d = b.position - a.position;
            const float length = glm::length(d);
            if (length <= 0.f) {
                continue;
            }
            const float age01 = std::clamp((frame.sim_time - b.time) / duration, 0.f, 1.f);
            const float alpha = 0.95f * std::pow(1.f - age01, 1.5f);
            segments.push_back(make_static(0.5f * (a.position + b.position), std::atan2(d.y, d.x),
                    {length + width, width}, glm::vec4{tint, alpha}));
        }
    }
}

void FieldView::build_coils(const Frame& frame) {
    auto& segments = emitter(coil_segments_).particles;
    auto& heads = emitter(coil_heads_).particles;
    segments.clear();
    heads.clear();
    const float width = kCoilWidthPx * frame.world_per_pixel;
    const glm::vec2 head_size{2.f * kHeadLengthPx * frame.world_per_pixel, 2.f * kHeadWidthPx * frame.world_per_pixel};
    for (const Coil& coil : frame.coils) {
        const auto point = [&](float angle) {
            return coil.centre + coil.radius * glm::vec3{std::cos(angle), std::sin(angle), 0.f};
        };
        for (int i = 0; i < kCoilDrawSegments; ++i) {
            const float a0 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / kCoilDrawSegments;
            const float a1 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i + 1) / kCoilDrawSegments;
            const glm::vec3 p0 = point(a0);
            const glm::vec3 p1 = point(a1);
            const glm::vec3 d = p1 - p0;
            segments.push_back(make_static(0.5f * (p0 + p1), std::atan2(d.y, d.x), {glm::length(d) + width, width},
                    kCoilColor));
        }
        const float direction = coil.centre_field >= 0.f ? 1.f : -1.f;
        for (int i = 0; i < kCoilArrows; ++i) {
            const float angle = 2.f * std::numbers::pi_v<float> * (static_cast<float>(i) + 0.5f) / kCoilArrows;
            // Counter-clockwise tangent is (−sin, cos); flipped when centre_field < 0.
            const glm::vec3 tangent = direction * glm::vec3{-std::sin(angle), std::cos(angle), 0.f};
            heads.push_back(make_static(point(angle), std::atan2(tangent.y, tangent.x), head_size,
                    glm::vec4{1.f, 0.85f, 0.6f, 1.f}));
        }
    }
}

}
