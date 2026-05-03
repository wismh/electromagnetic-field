#pragma once

#include <game/charge.h>
#include <game/electrostatics.h>

#include <vector>

namespace game {

// Owns the charges and advances them with kick-drift-kick leapfrog (velocity Verlet): symplectic,
// so total energy oscillates around its initial value instead of drifting. Fixed charges never move.
class Simulation {
public:
    explicit Simulation(FieldParams params = {});

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

    void step(float dt);

    [[nodiscard]] float total_energy() const;

private:
    void kick(float dt);
    void drift(float dt);

    std::vector<Charge> charges_;
    FieldParams params_;
};

}
