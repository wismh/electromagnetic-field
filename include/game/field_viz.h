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
    float seed_radius = 0.3f;
    float capture_radius = 0.25f;
    float step_fraction = 0.15f;
    float min_step = 0.02f;
    float max_step = 0.4f;
    int max_steps = 2000;
    Bounds bounds{{-50.f, -50.f}, {50.f, 50.f}};
};

using FieldLine = std::vector<glm::vec3>;

[[nodiscard]] std::vector<FieldLine> trace_field_lines(
        std::span<const Charge> charges, const FieldParams& params, const FieldLineOptions& options = {});

struct FieldSample {
    glm::vec3 position{0.f};
    glm::vec3 field{0.f};
};

[[nodiscard]] bool inside_charge_glyph(std::span<const Charge> charges, glm::vec3 point);

// Grid is locked to world multiples of spacing so arrows do not swim when the camera pans.
[[nodiscard]] std::vector<FieldSample> sample_field_grid(
        std::span<const Charge> charges, const FieldParams& params, const Bounds& bounds, float spacing);

[[nodiscard]] float field_strength01(float magnitude);

struct FlowParticle {
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    float age = 0.f;
    float lifetime = 1.f;
};

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
