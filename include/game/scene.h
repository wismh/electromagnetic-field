#pragma once

#include <game/charge.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace game {

// Must match kMaxCharges in assets/shaders/potential.shader.
inline constexpr std::size_t kMaxCharges = 32;

inline constexpr float kMinAbsCharge = 0.25f;
inline constexpr float kMaxAbsCharge = 5.f;
inline constexpr float kChargeStep = 0.25f;

enum class Preset {
    Dipole,
    LikePair,
    Quadrupole,
    Capacitor,
    Orbit,
};

[[nodiscard]] std::vector<Charge> make_preset(Preset preset);

// Radius of the drawn core disc in world units; grows with |q| so bigger charges read as bigger.
[[nodiscard]] float charge_radius(float q);

// Nearest charge whose core (slightly enlarged for easier grabbing) contains `point`.
[[nodiscard]] std::optional<std::size_t> pick_charge(std::span<const Charge> charges, glm::vec3 point);

// Changes |q| by `notches` * kChargeStep, keeping the sign and clamping to [kMinAbsCharge, kMaxAbsCharge].
[[nodiscard]] float step_charge_magnitude(float q, float notches);

}
