#include <game/electrostatics.h>

#include <glm/geometric.hpp>

#include <cmath>

namespace game {

glm::vec3 field_at(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params, std::size_t skip) {
    const float eps2 = params.softening * params.softening;
    glm::vec3 field{0.f};
    for (std::size_t i = 0; i < charges.size(); ++i) {
        if (i == skip) {
            continue;
        }
        const glm::vec3 d = point - charges[i].position;
        const float r2 = glm::dot(d, d) + eps2;
        if (r2 <= 0.f) {
            continue;
        }
        field += (params.k * charges[i].q / (r2 * std::sqrt(r2))) * d;
    }
    return field;
}

float potential_at(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params, std::size_t skip) {
    const float eps2 = params.softening * params.softening;
    float phi = 0.f;
    for (std::size_t i = 0; i < charges.size(); ++i) {
        if (i == skip) {
            continue;
        }
        const glm::vec3 d = point - charges[i].position;
        const float r2 = glm::dot(d, d) + eps2;
        if (r2 <= 0.f) {
            continue;
        }
        phi += params.k * charges[i].q / std::sqrt(r2);
    }
    return phi;
}

glm::vec3 force_on(std::span<const Charge> charges, std::size_t j, const FieldParams& params) {
    return charges[j].q * field_at(charges, charges[j].position, params, j);
}

float potential_energy(std::span<const Charge> charges, const FieldParams& params) {
    const float eps2 = params.softening * params.softening;
    float energy = 0.f;
    for (std::size_t i = 0; i < charges.size(); ++i) {
        for (std::size_t j = i + 1; j < charges.size(); ++j) {
            const glm::vec3 d = charges[j].position - charges[i].position;
            const float r2 = glm::dot(d, d) + eps2;
            if (r2 <= 0.f) {
                continue;
            }
            energy += params.k * charges[i].q * charges[j].q / std::sqrt(r2);
        }
    }
    return energy;
}

float kinetic_energy(std::span<const Charge> charges) {
    float energy = 0.f;
    for (const Charge& c : charges) {
        if (!c.fixed) {
            energy += 0.5f * c.mass * glm::dot(c.velocity, c.velocity);
        }
    }
    return energy;
}

}
