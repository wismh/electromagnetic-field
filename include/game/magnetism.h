#pragma once

#include <game/electrostatics.h>

#include <glm/vec3.hpp>

#include <span>

namespace game {

// A circular current loop lying in the plane of the screen. In that plane its field is purely along z and
// strongly non-uniform: it grows from the centre towards the wire, changes sign across it and falls off
// outside like a dipole field. Set by the field at its centre rather than by its current, so the coil looks
// the same whatever the speed of light slider says. A positive centre field means a counter-clockwise
// current.
struct Coil {
    glm::vec3 centre{0.f};
    float radius = 5.f;
    float centre_field = 2.f;
    // Softening of the wire: the field stays finite right at the conductor.
    float wire_radius = 0.2f;
};

// Bz of one coil at a point in its plane: Biot–Savart over kCoilSegments straight pieces of the loop,
// normalised so that the value at the centre is exactly `centre_field`.
[[nodiscard]] float coil_field_z(const Coil& coil, glm::vec3 point);

inline constexpr int kCoilSegments = 128;

// Bz at a point: the uniform external field, every coil and every moving charge except `skip`.
// Charges live in the xy plane, so B is perpendicular to it. The same softening as E keeps it finite.
[[nodiscard]] float magnetic_z(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params,
        std::size_t skip = kNoCharge, std::span<const Coil> coils = {});

// Exact in-plane rotation by φ = (q / m) Bz dt. Speed is unchanged. Bz is evaluated once from the
// positions and velocities at entry, and a charge does not feel its own field. Fixed and massless charges
// are left alone. Simulation::step applies it as two half rotations around the drift.
void rotate_magnetic(std::span<Charge> charges, const FieldParams& params, float dt, std::span<const Coil> coils = {});

}
