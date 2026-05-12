#pragma once

#include <game/charge.h>
#include <game/electrostatics.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <random>
#include <span>
#include <vector>

namespace game {

// Axis-aligned rectangle in the xy plane.
struct Bounds {
    glm::vec2 min{0.f};
    glm::vec2 max{0.f};

    [[nodiscard]] bool contains(glm::vec3 p) const {
        return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
    }
};

struct FieldLineOptions {
    float lines_per_unit_charge = 8.f;
    int min_lines_per_charge = 1;
    // Lines start this far from the charge centre and end once they get this close to another charge.
    float seed_radius = 0.3f;
    float capture_radius = 0.25f;
    // Step length adapts to the distance to the nearest charge: fine near charges, coarse far away.
    float step_fraction = 0.15f;
    float min_step = 0.02f;
    float max_step = 0.4f;
    int max_steps = 2000;
    Bounds bounds{{-50.f, -50.f}, {50.f, 50.f}};
};

// Polyline following E (from + charges forward, and from - charges backward when the line escapes
// to the bounds instead of ending on a + charge). Points are in world space with z = 0.
using FieldLine = std::vector<glm::vec3>;

[[nodiscard]] std::vector<FieldLine> trace_field_lines(
        std::span<const Charge> charges, const FieldParams& params, const FieldLineOptions& options = {});

struct FieldSample {
    glm::vec3 position{0.f};
    glm::vec3 field{0.f};
};

// E sampled on a grid aligned to world multiples of `spacing` (so arrows do not swim when the camera
// pans), skipping points that fall inside a drawn charge.
[[nodiscard]] std::vector<FieldSample> sample_field_grid(
        std::span<const Charge> charges, const FieldParams& params, const Bounds& bounds, float spacing);

// Maps |E| to [0, 1] on a log scale so both weak far fields and strong near fields stay readable.
[[nodiscard]] float field_strength01(float magnitude);

struct FlowParticle {
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    float age = 0.f;
    float lifetime = 1.f;
};

// Tracer particles advected along E. Speed grows with |E| on a log scale; particles that leave the
// bounds, reach a charge or outlive their lifetime respawn at a random point inside the bounds.
class FlowField {
public:
    explicit FlowField(std::size_t count = 900, unsigned seed = 1);

    void update(std::span<const Charge> charges, const FieldParams& params, const Bounds& bounds, float dt);

    [[nodiscard]] const std::vector<FlowParticle>& particles() const {
        return particles_;
    }

    float base_speed = 4.f;
    float capture_radius = 0.25f;

private:
    void respawn(FlowParticle& p, std::span<const Charge> charges, const Bounds& bounds, bool random_age);

    std::vector<FlowParticle> particles_;
    std::mt19937 rng_;
    bool initialized_ = false;
};

}
