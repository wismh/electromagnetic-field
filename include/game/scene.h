#pragma once

#include <game/charge.h>
#include <game/magnetism.h>

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
    Rutherford,
    Swarm,
    Cyclotron,
    ExBDrift,
    Coil,
};

[[nodiscard]] std::vector<Charge> make_preset(Preset preset);

// Field settings a scene needs besides its charges. Electrostatic scenes switch the Lorentz force off and
// clear the external field, so loading one never inherits magnetism from the previous scene.
struct SceneSettings {
    bool magnetic = false;
    float b_external = 0.f;
    std::vector<Coil> coils;
};

[[nodiscard]] SceneSettings scene_settings(Preset preset);

// Uniform Bz of the magnetic scenes. Exposed for the tests that check the motion against theory.
inline constexpr float kCyclotronField = 2.f;
inline constexpr float kExBDriftField = 1.5f;
inline constexpr Coil kSceneCoil{.radius = 5.f, .centre_field = 2.f};

// Nearest charge whose core (slightly enlarged for easier grabbing) contains `point`.
[[nodiscard]] std::optional<std::size_t> pick_charge(std::span<const Charge> charges, glm::vec3 point);

// Changes |q| by `notches` * kChargeStep, keeping the sign and clamping to [kMinAbsCharge, kMaxAbsCharge].
[[nodiscard]] float step_charge_magnitude(float q, float notches);

}
