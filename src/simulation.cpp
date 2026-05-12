#include <game/simulation.h>

#include <glm/geometric.hpp>

namespace game {

Simulation::Simulation(FieldParams params, DynamicsOptions dynamics) :
    params_(params),
    dynamics_(dynamics) {}

void Simulation::step(float dt) {
    kick(0.5f * dt);
    drift(dt);
    kick(0.5f * dt);
    if (dynamics_.collisions) {
        resolve_collisions();
    }
    limit_speed();
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

// Pairwise ball collisions: overlapping pairs are pushed apart along the contact normal (split by
// inverse mass, so fixed charges never move) and approaching pairs get an impulse that reverses
// their normal relative velocity scaled by the restitution. Momentum of free pairs is conserved.
void Simulation::resolve_collisions() {
    for (std::size_t i = 0; i < charges_.size(); ++i) {
        for (std::size_t j = i + 1; j < charges_.size(); ++j) {
            Charge& a = charges_[i];
            Charge& b = charges_[j];
            const float wa = (a.fixed || a.mass <= 0.f) ? 0.f : 1.f / a.mass;
            const float wb = (b.fixed || b.mass <= 0.f) ? 0.f : 1.f / b.mass;
            if (wa + wb <= 0.f) {
                continue;
            }
            const glm::vec3 d = b.position - a.position;
            const float distance = glm::length(d);
            const float contact = charge_radius(a.q) + charge_radius(b.q);
            if (distance >= contact || distance <= 1e-6f) {
                continue;
            }
            const glm::vec3 n = d / distance;
            const float overlap = contact - distance;
            a.position -= (overlap * wa / (wa + wb)) * n;
            b.position += (overlap * wb / (wa + wb)) * n;

            const float approach = glm::dot(b.velocity - a.velocity, n);
            if (approach < 0.f) {
                const float impulse = -(1.f + dynamics_.restitution) * approach / (wa + wb);
                a.velocity -= (impulse * wa) * n;
                b.velocity += (impulse * wb) * n;
            }
        }
    }
}

void Simulation::limit_speed() {
    for (Charge& c : charges_) {
        const float speed = glm::length(c.velocity);
        if (speed > dynamics_.max_speed) {
            c.velocity *= dynamics_.max_speed / speed;
        }
    }
}

}
