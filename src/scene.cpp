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
        case Preset::Cyclotron: {
            // Free charges in a uniform Bz, each started at the top (bottom for −q) of its circle moving
            // right, so the circles sit side by side. q/m = 0.5 for all, hence one period 2π m / (|q| B)
            // for every speed: the radius m v / (|q| B) grows with v, the period does not. Small charges
            // keep their mutual Coulomb pull negligible.
            constexpr float kQ = kMinAbsCharge;
            constexpr float kMass = 0.5f;
            const auto at = [&](float centre_x, float centre_y, float speed, float q) {
                const float radius = kMass * speed / (std::abs(q) * kCyclotronField);
                const float start_y = q > 0.f ? centre_y + radius : centre_y - radius;
                return Charge{.position = {centre_x, start_y, 0.f}, .velocity = {speed, 0.f, 0.f}, .q = q,
                        .mass = kMass};
            };
            return {
                    at(-7.f, 1.f, 1.f, kQ),
                    at(0.f, 1.f, 2.f, kQ),
                    at(8.f, 1.f, 3.f, kQ),
                    at(0.f, -6.f, 2.f, -kQ),
            };
        }
        case Preset::ExBDrift: {
            // Two fixed rows (a capacitor, E pointing down) in a uniform Bz. Charges released from rest
            // between them roll along cycloids and drift sideways at E / B; the drift direction E × B is the
            // same for both signs, only the loops turn the other way.
            std::vector<Charge> charges;
            for (int i = -4; i <= 4; ++i) {
                const float x = 2.f * static_cast<float>(i);
                charges.push_back(Charge{.position = {x, 3.f, 0.f}, .q = 1.f, .fixed = true});
                charges.push_back(Charge{.position = {x, -3.f, 0.f}, .q = -1.f, .fixed = true});
            }
            constexpr float kMass = 0.5f;
            charges.push_back(Charge{.position = {7.f, 1.f, 0.f}, .q = kMinAbsCharge, .mass = kMass});
            charges.push_back(Charge{.position = {3.5f, -0.5f, 0.f}, .q = kMinAbsCharge, .mass = kMass});
            charges.push_back(Charge{.position = {0.f, -1.2f, 0.f}, .q = -kMinAbsCharge, .mass = kMass});
            return charges;
        }
        case Preset::Coil: {
            // One coil (scene_settings) and four free charges with q/m = ±0.5. Each circles with a gyroradius
            // smaller than the distance over which the coil field changes, so the circle is slightly tighter
            // on its strong-field side and does not close: it slides sideways (grad-B drift) around the coil.
            // + and − drift in opposite directions. Inside the field grows towards the wire; outside it has
            // the opposite sign and falls off like a dipole field, the in-plane picture of a radiation belt.
            constexpr float kMass = 0.5f;
            constexpr float kQ = kMinAbsCharge;
            return {
                    Charge{.position = {3.f, 0.f, 0.f}, .velocity = {0.f, 1.f, 0.f}, .q = kQ, .mass = kMass},
                    Charge{.position = {-3.f, 0.f, 0.f}, .velocity = {0.f, 1.f, 0.f}, .q = -kQ, .mass = kMass},
                    Charge{.position = {7.5f, 0.f, 0.f}, .velocity = {0.f, 0.5f, 0.f}, .q = kQ, .mass = kMass},
                    Charge{.position = {-7.5f, 0.f, 0.f}, .velocity = {0.f, 0.5f, 0.f}, .q = -kQ, .mass = kMass},
            };
        }
    }
    return {};
}

SceneSettings scene_settings(Preset preset) {
    switch (preset) {
        case Preset::Cyclotron:
            return SceneSettings{.magnetic = true, .b_external = kCyclotronField};
        case Preset::ExBDrift:
            return SceneSettings{.magnetic = true, .b_external = kExBDriftField};
        case Preset::Coil:
            return SceneSettings{.magnetic = true, .coils = {kSceneCoil}};
        default:
            return SceneSettings{};
    }
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
