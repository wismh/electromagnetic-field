#include <game/scene.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>

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
    }
    return {};
}

float charge_radius(float q) {
    return 0.2f + 0.15f * std::sqrt(std::abs(q));
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
