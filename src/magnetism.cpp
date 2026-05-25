#include <game/magnetism.h>

#include <glm/geometric.hpp>

#include <cmath>
#include <numbers>
#include <vector>

namespace game {

namespace {

// Sum over the loop of (dl × r)_z / (r² + w²)^{3/2} for a unit counter-clockwise current. Each piece is a
// chord of the circle with the field taken at its midpoint.
float loop_sum(const Coil& coil, glm::vec3 point) {
    const float w2 = coil.wire_radius * coil.wire_radius;
    float sum = 0.f;
    for (int i = 0; i < kCoilSegments; ++i) {
        const float a0 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / kCoilSegments;
        const float a1 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i + 1) / kCoilSegments;
        const float am = 0.5f * (a0 + a1);
        const glm::vec3 source = coil.centre + coil.radius * glm::vec3{std::cos(am), std::sin(am), 0.f};
        const glm::vec3 dl = coil.radius * glm::vec3{std::cos(a1) - std::cos(a0), std::sin(a1) - std::sin(a0), 0.f};
        const glm::vec3 r = point - source;
        const float r2 = glm::dot(r, r) + w2;
        sum += (dl.x * r.y - dl.y * r.x) / (r2 * std::sqrt(r2));
    }
    return sum;
}

}

float coil_field_z(const Coil& coil, glm::vec3 point) {
    const float at_centre = loop_sum(coil, coil.centre);
    if (at_centre == 0.f) {
        return 0.f;
    }
    return coil.centre_field * loop_sum(coil, point) / at_centre;
}

float magnetic_z(std::span<const Charge> charges, glm::vec3 point, const FieldParams& params, std::size_t skip,
        std::span<const Coil> coils) {
    const float eps2 = params.softening * params.softening;
    const float coupling = magnetic_coupling(params);
    float field_z = params.b_external;
    for (const Coil& coil : coils) {
        field_z += coil_field_z(coil, point);
    }
    for (std::size_t i = 0; i < charges.size(); ++i) {
        if (i == skip) {
            continue;
        }
        const Charge& c = charges[i];
        const glm::vec3 d = point - c.position;
        const float r2 = glm::dot(d, d) + eps2;
        if (r2 <= 0.f) {
            continue;
        }
        // (v × d)_z. d points from the source to the sample, matching v × r-hat / r^2.
        const float perpendicular = c.velocity.x * d.y - c.velocity.y * d.x;
        field_z += coupling * c.q * perpendicular / (r2 * std::sqrt(r2));
    }
    return field_z;
}

void rotate_magnetic(std::span<Charge> charges, const FieldParams& params, float dt, std::span<const Coil> coils) {
    std::vector<float> field_z(charges.size(), 0.f);
    for (std::size_t j = 0; j < charges.size(); ++j) {
        const Charge& c = charges[j];
        if (c.fixed || c.mass <= 0.f) {
            continue;
        }
        field_z[j] = magnetic_z(charges, c.position, params, j, coils);
    }
    for (std::size_t j = 0; j < charges.size(); ++j) {
        Charge& c = charges[j];
        if (c.fixed || c.mass <= 0.f) {
            continue;
        }
        // Clockwise when q Bz > 0: dv/dt = (q/m) v × B with B along +z.
        const float phi = (c.q / c.mass) * field_z[j] * dt;
        const float cosine = std::cos(phi);
        const float sine = std::sin(phi);
        const float vx = c.velocity.x;
        const float vy = c.velocity.y;
        c.velocity.x = vx * cosine + vy * sine;
        c.velocity.y = -vx * sine + vy * cosine;
    }
}

}
