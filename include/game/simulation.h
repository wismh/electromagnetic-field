#pragma once

#include <game/charge.h>
#include <game/electrostatics.h>
#include <game/magnetism.h>

#include <vector>

namespace game {

inline constexpr float kMaxLightFraction = 0.9f;

struct DynamicsOptions {
    // Charges are balls of radius charge_radius(q) that bounce off each other instead of passing
    // through (opposite charges would otherwise fall through each other and fly apart).
    bool collisions = true;
    // Lorentz force from Bz (the other charges and the uniform external field). Off leaves the step
    // identical to the electric velocity Verlet.
    bool magnetic = false;
    // 1 = perfectly elastic, 0 = the approaching relative velocity is removed completely.
    float restitution = 0.8f;
    // Hard cap on |v| so a near miss can never launch a charge off to infinity. The step also keeps every
    // speed below kMaxLightFraction · c, where the quasi-static model still means something.
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

    // Current loops of the scene. Their field acts only while the Lorentz force is on.
    std::vector<Coil>& coils() {
        return coils_;
    }

    const std::vector<Coil>& coils() const {
        return coils_;
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
    std::vector<Coil> coils_;
    FieldParams params_;
    DynamicsOptions dynamics_;
};

}
