#pragma once

#include <glm/vec3.hpp>

namespace game {

// Point charge. Vectors are 3D with z = 0 so the 2D plane stays self-consistent once magnetic
// fields (B along z) arrive, and a later move to 2.5D/3D does not change the physics core.
struct Charge {
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    float q = 1.f;
    float mass = 1.f;  // ignored when fixed
    bool fixed = false;
};

}
