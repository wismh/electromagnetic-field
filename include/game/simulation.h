#pragma once

#include <game/charge.h>
#include <game/electrostatics.h>
#include <game/magnetism.h>

#include <vector>

namespace game {

inline constexpr float kMaxLightFraction = 0.9f;

struct DynamicsOptions {
    bool collisions = true;
    bool magnetic = false;
    float restitution = 0.8f;
    float max_speed = 30.f;
};

// Velocity Verlet. Fixed charges never move.
class Simulation {
public:
    explicit Simulation(FieldParams params = {}, DynamicsOptions dynamics = {});

    std::vector<Charge>& charges() {
        return charges_;
    }

    const std::vector<Charge>& charges() const {
        return charges_;
    }

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
