#pragma once

#include <game/charge.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <limits>
#include <span>

namespace game {

// Simulation units: k = 1 by default instead of 8.99e9 so typical values stay near 1.
struct FieldParams {
    float k = 1.f;
    // Plummer softening length: r^2 is replaced by r^2 + softening^2 so the field stays finite
    // at r -> 0. Field, potential and energy all use the same softened form, so F = -grad U
    // holds exactly and the integrator conserves energy.
    float softening = 0.25f;
    // Speed of light in simulation units. It fixes the magnetic constant through μ₀/4π = k / c², the same
    // relation as μ₀ε₀ = 1/c² in SI, so magnetic forces between moving charges are (v/c)² of the Coulomb
    // force, as in nature. The quasi-static model (instant fields, no induction) is valid only for v ≪ c.
    float light_speed = 20.f;
    // Uniform field along +z. Motion uses it only when the Lorentz-force flag is on.
    float b_external = 0.f;
};

inline constexpr std::size_t kNoCharge = std::numeric_limits<std::size_t>::max();

// μ₀/4π in simulation units: k / c².
[[nodiscard]] inline float magnetic_coupling(const FieldParams& params) {
    return params.light_speed > 0.f ? params.k / (params.light_speed * params.light_speed) : 0.f;
}

// E(p) = sum_i k * q_i * (p - r_i) / (|p - r_i|^2 + eps^2)^(3/2), skipping charge `skip`.
[[nodiscard]] glm::vec3 field_at(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params,
        std::size_t skip = kNoCharge);

// phi(p) = sum_i k * q_i / sqrt(|p - r_i|^2 + eps^2), skipping charge `skip`.
[[nodiscard]] float potential_at(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params,
        std::size_t skip = kNoCharge);

// F_j = q_j * E(r_j) from every charge except j itself.
[[nodiscard]] glm::vec3 force_on(std::span<const Charge> charges, std::size_t j, const FieldParams& params);

// U = sum_{i<j} k * q_i * q_j / sqrt(r_ij^2 + eps^2).
[[nodiscard]] float potential_energy(std::span<const Charge> charges, const FieldParams& params);

// Sum of m * v^2 / 2 over free charges.
[[nodiscard]] float kinetic_energy(std::span<const Charge> charges);

}
