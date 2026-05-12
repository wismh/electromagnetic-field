#pragma once

#include <game/charge.h>
#include <game/electrostatics.h>

#include <vector>

namespace game {

struct DynamicsOptions {
    // Charges are balls of radius charge_radius(q) that bounce off each other instead of passing
    // through (opposite charges would otherwise fall through each other and fly apart).
    bool collisions = true;
    // 1 = perfectly elastic, 0 = the approaching relative velocity is removed completely.
    float restitution = 0.8f;
    // Hard cap on |v| so a near miss can never launch a charge off to infinity.
    float max_speed = 30.f;
};

// Owns the charges and advances them with kick-drift-kick leapfrog (velocity Verlet): symplectic,
// so total energy oscillates around its initial value instead of drifting. Fixed charges never move.
class Simulation {
public:
    explicit Simulation(FieldParams params = {}, DynamicsOptions dynamics = {});

    std::vector<Charge>& charges() {
        return charges_;
    }

    const std::vector<Charge>& charges() const {
        return charges_;
    }

    const FieldParams& params() const {
        return params_;
    }

    void set_params(const FieldParams& params) {
        params_ = params;
    }

    const DynamicsOptions& dynamics() const {
        return dynamics_;
    }

    void set_dynamics(const DynamicsOptions& dynamics) {
        dynamics_ = dynamics;
    }

    void step(float dt);

    [[nodiscard]] float total_energy() const;

private:
    void kick(float dt);
    void drift(float dt);
    void resolve_collisions();
    void limit_speed();

    std::vector<Charge> charges_;
    FieldParams params_;
    DynamicsOptions dynamics_;
};

}
