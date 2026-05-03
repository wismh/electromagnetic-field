#include <game/simulation.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <cmath>

namespace game {
namespace {

// Two opposite unit charges with unit masses, 2 apart: each circles the centre of mass at radius 1.
// Circular orbit: m * v^2 / r = k * q^2 / d^2  ->  v^2 = 1 * 1 / 4  ->  v = 0.5.
Simulation make_orbit() {
    Simulation sim(FieldParams{.k = 1.f, .softening = 0.f});
    sim.charges().push_back(Charge{.position = {-1.f, 0.f, 0.f}, .velocity = {0.f, -0.5f, 0.f}, .q = 1.f});
    sim.charges().push_back(Charge{.position = {1.f, 0.f, 0.f}, .velocity = {0.f, 0.5f, 0.f}, .q = -1.f});
    return sim;
}

TEST(SimulationTest, CircularOrbitKeepsRadius) {
    Simulation sim = make_orbit();

    for (int i = 0; i < 20000; ++i) {
        sim.step(1e-3f);
        const float d = glm::length(sim.charges()[1].position - sim.charges()[0].position);
        ASSERT_NEAR(d, 2.f, 1e-2f) << "step " << i;
    }
}

TEST(SimulationTest, EnergyIsConserved) {
    Simulation sim = make_orbit();
    const float e0 = sim.total_energy();

    for (int i = 0; i < 20000; ++i) {
        sim.step(1e-3f);
    }

    EXPECT_NEAR(sim.total_energy(), e0, 1e-3f * std::abs(e0));
}

TEST(SimulationTest, MomentumIsConservedWithoutFixedCharges) {
    Simulation sim(FieldParams{.k = 1.f, .softening = 0.1f});
    sim.charges().push_back(Charge{.position = {0.f, 0.f, 0.f}, .velocity = {0.2f, 0.f, 0.f}, .q = 1.f, .mass = 1.f});
    sim.charges().push_back(Charge{.position = {1.f, 1.f, 0.f}, .q = -2.f, .mass = 3.f});
    sim.charges().push_back(Charge{.position = {-1.f, 2.f, 0.f}, .q = 0.5f, .mass = 0.5f});
    const auto momentum = [&sim] {
        glm::vec3 p{0.f};
        for (const Charge& c : sim.charges()) {
            p += c.mass * c.velocity;
        }
        return p;
    };
    const glm::vec3 p0 = momentum();

    for (int i = 0; i < 2000; ++i) {
        sim.step(1e-3f);
    }

    EXPECT_NEAR(glm::length(momentum() - p0), 0.f, 1e-4f);
}

TEST(SimulationTest, FixedChargeDoesNotMove) {
    Simulation sim;
    sim.charges().push_back(Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f, .fixed = true});
    sim.charges().push_back(Charge{.position = {1.f, 0.f, 0.f}, .q = 1.f});

    for (int i = 0; i < 100; ++i) {
        sim.step(1e-2f);
    }

    EXPECT_EQ(sim.charges()[0].position, glm::vec3(0.f));
    EXPECT_EQ(sim.charges()[0].velocity, glm::vec3(0.f));
    EXPECT_GT(sim.charges()[1].position.x, 1.f);
}

TEST(SimulationTest, PlanarMotionStaysInPlane) {
    Simulation sim;
    sim.charges().push_back(Charge{.position = {0.f, 0.f, 0.f}, .velocity = {0.1f, 0.3f, 0.f}, .q = 1.f});
    sim.charges().push_back(Charge{.position = {1.f, 0.5f, 0.f}, .q = -1.f});
    sim.charges().push_back(Charge{.position = {-0.5f, 1.f, 0.f}, .q = 2.f, .fixed = true});

    for (int i = 0; i < 1000; ++i) {
        sim.step(1e-3f);
    }

    for (const Charge& c : sim.charges()) {
        EXPECT_EQ(c.position.z, 0.f);
        EXPECT_EQ(c.velocity.z, 0.f);
    }
}

}
}
