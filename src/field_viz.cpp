#include <game/field_viz.h>

#include <game/scene.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

namespace game {
namespace {

constexpr float kMinFieldForDirection = 1e-8f;
// |E| at which field_strength01 saturates and the log scale's knee.
constexpr float kStrengthKnee = 0.02f;
constexpr float kStrengthMax = 20.f;
// Share of respawning flow tracers that start next to a + charge rather than anywhere in view.
constexpr float kSourceSpawnFraction = 0.35f;

struct NearestCharge {
    std::size_t index = kNoCharge;
    float distance = 0.f;
};

NearestCharge nearest_charge(std::span<const Charge> charges, glm::vec3 p) {
    NearestCharge best;
    for (std::size_t i = 0; i < charges.size(); ++i) {
        const float d = glm::length(charges[i].position - p);
        if (best.index == kNoCharge || d < best.distance) {
            best = NearestCharge{.index = i, .distance = d};
        }
    }
    return best;
}

std::optional<glm::vec3> direction_at(std::span<const Charge> charges, const FieldParams& params, glm::vec3 p,
        float sign) {
    const glm::vec3 e = field_at(charges, p, params);
    const float magnitude = glm::length(e);
    if (magnitude < kMinFieldForDirection) {
        return std::nullopt;
    }
    return (sign / magnitude) * e;
}

enum class LineEnd {
    Charge,
    Escaped,  // left the bounds, hit a null point or ran out of steps
};

struct TraceResult {
    FieldLine points;
    LineEnd end = LineEnd::Escaped;
    std::size_t end_charge = kNoCharge;
};

// Integrates dp/ds = sign * E/|E| with RK4 from `start`, which lies near charge `origin`.
TraceResult trace_from(std::span<const Charge> charges, const FieldParams& params, const FieldLineOptions& options,
        glm::vec3 start, std::size_t origin, float sign) {
    TraceResult result;
    result.points.push_back(start);
    glm::vec3 p = start;
    for (int step = 0; step < options.max_steps; ++step) {
        const NearestCharge nearest = nearest_charge(charges, p);
        if (nearest.index != origin && nearest.distance < options.capture_radius) {
            result.points.push_back(charges[nearest.index].position);
            result.end = LineEnd::Charge;
            result.end_charge = nearest.index;
            return result;
        }
        if (!options.bounds.contains(p)) {
            return result;
        }
        const float h = std::clamp(options.step_fraction * nearest.distance, options.min_step, options.max_step);

        const auto k1 = direction_at(charges, params, p, sign);
        const auto k2 = k1 ? direction_at(charges, params, p + 0.5f * h * *k1, sign) : std::nullopt;
        const auto k3 = k2 ? direction_at(charges, params, p + 0.5f * h * *k2, sign) : std::nullopt;
        const auto k4 = k3 ? direction_at(charges, params, p + h * *k3, sign) : std::nullopt;
        if (!k4) {
            return result;
        }
        p += (h / 6.f) * (*k1 + 2.f * *k2 + 2.f * *k3 + *k4);
        p.z = 0.f;
        result.points.push_back(p);
    }
    return result;
}

int line_count(float q, const FieldLineOptions& options) {
    const int scaled = static_cast<int>(std::lround(options.lines_per_unit_charge * std::abs(q)));
    return std::max(options.min_lines_per_charge, scaled);
}

}

std::vector<FieldLine> trace_field_lines(
        std::span<const Charge> charges, const FieldParams& params, const FieldLineOptions& options) {
    std::vector<FieldLine> lines;
    for (std::size_t i = 0; i < charges.size(); ++i) {
        const Charge& c = charges[i];
        if (c.q == 0.f) {
            continue;
        }
        // Forward from + charges; backward (against E) from - charges.
        const float sign = c.q > 0.f ? 1.f : -1.f;
        const int count = line_count(c.q, options);
        for (int k = 0; k < count; ++k) {
            const float angle = 2.f * std::numbers::pi_v<float> * (static_cast<float>(k) + 0.5f) /
                    static_cast<float>(count);
            const glm::vec3 start = c.position + options.seed_radius * glm::vec3{std::cos(angle), std::sin(angle), 0.f};
            TraceResult traced = trace_from(charges, params, options, start, i, sign);
            // A backward line that lands on a + charge duplicates one already traced forward from it.
            if (sign < 0.f && traced.end == LineEnd::Charge && charges[traced.end_charge].q > 0.f) {
                continue;
            }
            if (traced.points.size() >= 2) {
                lines.push_back(std::move(traced.points));
            }
        }
    }
    return lines;
}

std::vector<FieldSample> sample_field_grid(
        std::span<const Charge> charges, const FieldParams& params, const Bounds& bounds, float spacing) {
    std::vector<FieldSample> samples;
    if (spacing <= 0.f) {
        return samples;
    }
    const float x0 = std::ceil(bounds.min.x / spacing) * spacing;
    const float y0 = std::ceil(bounds.min.y / spacing) * spacing;
    for (float y = y0; y <= bounds.max.y; y += spacing) {
        for (float x = x0; x <= bounds.max.x; x += spacing) {
            const glm::vec3 p{x, y, 0.f};
            const bool inside_charge = std::any_of(charges.begin(), charges.end(), [&](const Charge& c) {
                return glm::length(c.position - p) < 1.5f * charge_radius(c.q);
            });
            if (!inside_charge) {
                samples.push_back(FieldSample{.position = p, .field = field_at(charges, p, params)});
            }
        }
    }
    return samples;
}

float field_strength01(float magnitude) {
    const float t = std::log1p(magnitude / kStrengthKnee) / std::log1p(kStrengthMax / kStrengthKnee);
    return std::clamp(t, 0.f, 1.f);
}

FlowField::FlowField(std::size_t count, unsigned seed) :
    particles_(count),
    rng_(seed) {}

void FlowField::update(std::span<const Charge> charges, const FieldParams& params, const Bounds& bounds, float dt) {
    if (!initialized_) {
        for (FlowParticle& p : particles_) {
            respawn(p, charges, bounds, true);
        }
        initialized_ = true;
    }
    const auto velocity_at = [&](glm::vec3 p) {
        const glm::vec3 e = field_at(charges, p, params);
        const float magnitude = glm::length(e);
        if (magnitude < kMinFieldForDirection) {
            return glm::vec3{0.f};
        }
        const float speed = base_speed * (0.15f + 1.35f * field_strength01(magnitude));
        return (speed / magnitude) * e;
    };

    for (FlowParticle& p : particles_) {
        p.age += dt;
        // Midpoint step: cheap, and keeps particles on curved field lines far better than Euler.
        const glm::vec3 v1 = velocity_at(p.position);
        const glm::vec3 v2 = velocity_at(p.position + 0.5f * dt * v1);
        p.velocity = v2;
        p.position += dt * v2;
        p.position.z = 0.f;

        const NearestCharge nearest = nearest_charge(charges, p.position);
        const bool captured = nearest.index != kNoCharge && nearest.distance < capture_radius;
        if (p.age >= p.lifetime || captured || !bounds.contains(p.position)) {
            respawn(p, charges, bounds, false);
        }
    }
}

void FlowField::respawn(FlowParticle& p, std::span<const Charge> charges, const Bounds& bounds, bool random_age) {
    std::uniform_real_distribution<float> unit(0.f, 1.f);
    std::uniform_real_distribution<float> life(1.5f, 4.f);
    p.velocity = glm::vec3{0.f};
    p.lifetime = life(rng_);
    // Staggered ages on the first fill so the whole population does not fade in and out in sync.
    p.age = random_age ? unit(rng_) * p.lifetime : 0.f;

    // Field lines start on + charges, so uniform spawning alone leaves sources empty (everything
    // streams away from them). Part of the population is born just outside a + charge instead,
    // picked with probability proportional to q.
    float positive_total = 0.f;
    for (const Charge& c : charges) {
        positive_total += std::max(c.q, 0.f);
    }
    if (positive_total > 0.f && unit(rng_) < kSourceSpawnFraction) {
        float pick = unit(rng_) * positive_total;
        for (const Charge& c : charges) {
            if (c.q <= 0.f) {
                continue;
            }
            pick -= c.q;
            if (pick <= 0.f) {
                const float angle = 2.f * std::numbers::pi_v<float> * unit(rng_);
                const float radius = capture_radius * (1.5f + 1.5f * unit(rng_));
                p.position = c.position + radius * glm::vec3{std::cos(angle), std::sin(angle), 0.f};
                if (bounds.contains(p.position)) {
                    return;
                }
                break;
            }
        }
    }

    std::uniform_real_distribution<float> ux(bounds.min.x, bounds.max.x);
    std::uniform_real_distribution<float> uy(bounds.min.y, bounds.max.y);
    // A few tries to avoid spawning inside a charge; falling back to the last try is harmless.
    for (int attempt = 0; attempt < 4; ++attempt) {
        p.position = {ux(rng_), uy(rng_), 0.f};
        const NearestCharge nearest = nearest_charge(charges, p.position);
        if (nearest.index == kNoCharge || nearest.distance > 2.f * capture_radius) {
            break;
        }
    }
}

}
