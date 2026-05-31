#pragma once

#include <game/charge.h>
#include <game/magnetism.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace game {

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

// Electrostatic scenes clear B so a load never inherits the previous scene's field.
struct SceneSettings {
    bool magnetic = false;
    float b_external = 0.f;
    std::vector<Coil> coils;
};

[[nodiscard]] SceneSettings scene_settings(Preset preset);

inline constexpr float kCyclotronField = 2.f;
inline constexpr float kExBDriftField = 1.5f;
inline constexpr Coil kSceneCoil{.radius = 5.f, .centre_field = 2.f};

[[nodiscard]] std::optional<std::size_t> pick_charge(std::span<const Charge> charges, glm::vec3 point);

[[nodiscard]] float step_charge_magnitude(float q, float notches);

}
