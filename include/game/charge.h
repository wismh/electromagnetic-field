#pragma once

#include <glm/vec3.hpp>

#include <cmath>
#include <cstdint>

namespace game {

// Point charge. Vectors are 3D with z = 0 so the 2D plane stays self-consistent once magnetic
// fields (B along z) arrive, and a later move to 2.5D/3D does not change the physics core.
struct Charge {
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    float q = 1.f;
    float mass = 1.f;  // ignored when fixed
    bool fixed = false;
    // Stable identity for per-charge state kept outside the vector (trails). 0 = not assigned yet.
    std::uint32_t id = 0;
};

// Radius of the charged ball in world units: used for drawing, picking and collisions. Grows with
// |q| so bigger charges read as bigger.
[[nodiscard]] inline float charge_radius(float q) {
    return 0.2f + 0.15f * std::sqrt(std::abs(q));
}

}
