#pragma once

#include <game/electrostatics.h>

#include <glm/vec3.hpp>

#include <span>

namespace game {

// In-plane loop. Field is set by centre_field, not current, so it ignores the c slider.
// Positive centre_field is a counter-clockwise current.
struct Coil {
    glm::vec3 centre{0.f};
    float radius = 5.f;
    float centre_field = 2.f;
    float wire_radius = 0.2f;
};

// Biot–Savart over kCoilSegments chords, normalised to centre_field at the centre.
[[nodiscard]] float coil_field_z(const Coil& coil, glm::vec3 point);

inline constexpr int kCoilSegments = 128;

[[nodiscard]] float magnetic_z(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params,
        std::size_t skip = kNoCharge, std::span<const Coil> coils = {});

// In-plane rotation by φ = (q/m) Bz dt. Speed is unchanged. A charge skips its own field.
void rotate_magnetic(std::span<Charge> charges, const FieldParams& params, float dt, std::span<const Coil> coils = {});

}
