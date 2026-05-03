#include <game/simulation.h>

namespace game {

Simulation::Simulation(FieldParams params) :
    params_(params) {}

void Simulation::step(float dt) {
    kick(0.5f * dt);
    drift(dt);
    kick(0.5f * dt);
}

float Simulation::total_energy() const {
    return kinetic_energy(charges_) + potential_energy(charges_, params_);
}

// Forces depend only on positions, so updating velocities in place does not affect later forces.
void Simulation::kick(float dt) {
    for (std::size_t j = 0; j < charges_.size(); ++j) {
        Charge& c = charges_[j];
        if (c.fixed || c.mass <= 0.f) {
            continue;
        }
        c.velocity += (dt / c.mass) * force_on(charges_, j, params_);
    }
}

void Simulation::drift(float dt) {
    for (Charge& c : charges_) {
        if (!c.fixed) {
            c.position += dt * c.velocity;
        }
    }
}

}
