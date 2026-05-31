#include <game/simulation.h>

#include <game/magnetism.h>

#include <glm/geometric.hpp>

#include <algorithm>

namespace game {

Simulation::Simulation(FieldParams params, DynamicsOptions dynamics) :
    params_(params),
    dynamics_(dynamics) {}

// Strang split: half kick, half rotation, drift, half rotation, half kick.
// One full rotation before the drift lets K + U creep.
void Simulation::step(float dt) {
    kick(0.5f * dt);
    if (dynamics_.magnetic) {
        rotate_magnetic(charges_, params_, 0.5f * dt, coils_);
    }
    drift(dt);
    if (dynamics_.magnetic) {
        rotate_magnetic(charges_, params_, 0.5f * dt, coils_);
    }
    kick(0.5f * dt);
    if (dynamics_.collisions) {
        resolve_collisions();
    }
    limit_speed();
}

float Simulation::total_energy() const {
    return kinetic_energy(charges_) + potential_energy(charges_, params_);
}

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

// Overlap is split by inverse mass (fixed charges stay put). Approaching pairs bounce.
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
    float cap = dynamics_.max_speed;
    if (params_.light_speed > 0.f) {
        cap = std::min(cap, kMaxLightFraction * params_.light_speed);
    }
    for (Charge& c : charges_) {
        const float speed = glm::length(c.velocity);
        if (speed > cap) {
            c.velocity *= cap / speed;
        }
    }
}

}
