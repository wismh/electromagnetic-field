#include <game/scene.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace game {
namespace {

constexpr float kPickSlack = 1.3f;

}

std::vector<Charge> make_preset(Preset preset) {
    switch (preset) {
        case Preset::Dipole:
            return {
                    Charge{.position = {-3.f, 0.f, 0.f}, .q = 1.f, .fixed = true},
                    Charge{.position = {3.f, 0.f, 0.f}, .q = -1.f, .fixed = true},
            };
        case Preset::LikePair:
            return {
                    Charge{.position = {-3.f, 0.f, 0.f}, .q = 1.f, .fixed = true},
                    Charge{.position = {3.f, 0.f, 0.f}, .q = 1.f, .fixed = true},
            };
        case Preset::Quadrupole:
            return {
                    Charge{.position = {-3.f, 3.f, 0.f}, .q = 1.f, .fixed = true},
                    Charge{.position = {3.f, 3.f, 0.f}, .q = -1.f, .fixed = true},
                    Charge{.position = {3.f, -3.f, 0.f}, .q = 1.f, .fixed = true},
                    Charge{.position = {-3.f, -3.f, 0.f}, .q = -1.f, .fixed = true},
            };
        case Preset::Capacitor: {
            std::vector<Charge> charges;
            for (int i = -3; i <= 3; ++i) {
                const float x = 2.f * static_cast<float>(i);
                charges.push_back(Charge{.position = {x, 3.f, 0.f}, .q = 0.5f, .fixed = true});
                charges.push_back(Charge{.position = {x, -3.f, 0.f}, .q = -0.5f, .fixed = true});
            }
            return charges;
        }
        case Preset::Orbit: {
            // Circular orbit around a fixed centre: m * v^2 / r = k * |Q * q| / r^2 with k = m = 1.
            constexpr float kCentreQ = 2.f;
            constexpr float kSatelliteQ = -0.5f;
            constexpr float kRadius = 3.f;
            const float speed = std::sqrt(kCentreQ * -kSatelliteQ / kRadius);
            return {
                    Charge{.position = {0.f, 0.f, 0.f}, .q = kCentreQ, .fixed = true},
                    Charge{.position = {0.f, kRadius, 0.f}, .velocity = {speed, 0.f, 0.f}, .q = kSatelliteQ},
            };
        }
        case Preset::Rutherford: {
            // Light + projectiles fired at a heavy fixed + nucleus with a spread of impact parameters:
            // the trails trace Rutherford's hyperbolic scattering orbits.
            std::vector<Charge> charges{Charge{.position = {0.f, 0.f, 0.f}, .q = 4.f, .fixed = true}};
            for (int i = -6; i <= 6; ++i) {
                const float impact = 0.75f * static_cast<float>(i);
                charges.push_back(Charge{.position = {-14.f, impact, 0.f}, .velocity = {2.f, 0.f, 0.f},
                        .q = kMinAbsCharge, .mass = 1.f});
            }
            return charges;
        }
        case Preset::Swarm: {
            // Free charges of alternating sign on two rings, released from rest: they pull together,
            // collide and settle into bound clusters.
            std::vector<Charge> charges;
            for (int ring = 0; ring < 2; ++ring) {
                const int count = ring == 0 ? 6 : 10;
                const float radius = ring == 0 ? 3.f : 7.f;
                for (int k = 0; k < count; ++k) {
                    const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(k) /
                            static_cast<float>(count) + 0.3f * static_cast<float>(ring);
                    charges.push_back(Charge{
                            .position = {radius * std::cos(angle), radius * std::sin(angle), 0.f},
                            .q = (k + ring) % 2 == 0 ? 1.f : -1.f,
                    });
                }
            }
            return charges;
        }
    }
    return {};
}

std::optional<std::size_t> pick_charge(std::span<const Charge> charges, glm::vec3 point) {
    std::optional<std::size_t> best;
    float best_distance = 0.f;
    for (std::size_t i = 0; i < charges.size(); ++i) {
        const float distance = glm::length(charges[i].position - point);
        if (distance > kPickSlack * charge_radius(charges[i].q)) {
            continue;
        }
        if (!best || distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

float step_charge_magnitude(float q, float notches) {
    const float sign = q < 0.f ? -1.f : 1.f;
    const float magnitude = std::clamp(std::abs(q) + notches * kChargeStep, kMinAbsCharge, kMaxAbsCharge);
    return sign * magnitude;
}

}
