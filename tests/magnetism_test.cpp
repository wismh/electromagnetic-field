#include <game/electrostatics.h>
#include <game/magnetism.h>
#include <game/simulation.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace game {
namespace {

// c = 1 makes μ₀/4π = k / c² = 1, so the expected Bz values below are the bare Biot–Savart numbers.
constexpr FieldParams kExact{.k = 1.f, .softening = 0.f, .light_speed = 1.f};

Charge moving_right() {
    return Charge{.position = {0.f, 0.f, 0.f}, .velocity = {1.f, 0.f, 0.f}, .q = 1.f};
}

TEST(MagnetismTest, PositiveChargeAboveHasPositiveBz) {
    const std::array charges{moving_right()};

    const float above = magnetic_z(charges, {0.f, 2.f, 0.f}, kExact);

    EXPECT_NEAR(above, 0.25f, 1e-6f);
}

TEST(MagnetismTest, FieldVanishesOnTheVelocityAxis) {
    const std::array charges{moving_right()};

    EXPECT_NEAR(magnetic_z(charges, {2.f, 0.f, 0.f}, kExact), 0.f, 1e-6f);
    EXPECT_NEAR(magnetic_z(charges, {-3.f, 0.f, 0.f}, kExact), 0.f, 1e-6f);
}

TEST(MagnetismTest, ChargeDoesNotSourceItsOwnField) {
    const std::array charges{moving_right()};

    EXPECT_NEAR(magnetic_z(charges, charges[0].position, kExact, 0), 0.f, 1e-6f);
}

TEST(MagnetismTest, ExternalFieldIsUniform) {
    const FieldParams params{.k = 1.f, .softening = 0.f, .b_external = 1.5f};
    const std::array<Charge, 1> at_rest{Charge{.position = {1.f, 1.f, 0.f}, .q = 4.f}};

    EXPECT_NEAR(magnetic_z({}, {0.f, 0.f, 0.f}, params), 1.5f, 1e-6f);
    EXPECT_NEAR(magnetic_z({}, {4.f, -3.f, 0.f}, params), 1.5f, 1e-6f);
    EXPECT_NEAR(magnetic_z(at_rest, {8.f, 8.f, 0.f}, params), 1.5f, 1e-6f);
    // On the velocity axis the moving charge adds nothing, so only the external field remains.
    const std::array moving{moving_right()};
    EXPECT_NEAR(magnetic_z(moving, {2.f, 0.f, 0.f}, params), 1.5f, 1e-6f);
}

TEST(MagnetismTest, RotationIsClockwiseAndPreservesSpeed) {
    std::array charges{moving_right()};
    const FieldParams params{.softening = 0.f, .b_external = 1.f};

    rotate_magnetic(charges, params, 0.1f);

    EXPECT_LT(charges[0].velocity.y, 0.f);
    EXPECT_NEAR(glm::length(charges[0].velocity), 1.f, 1e-5f);
    EXPECT_EQ(charges[0].velocity.z, 0.f);
}

TEST(MagnetismTest, NegativeChargeTurnsTheOtherWay) {
    std::array charges{Charge{.velocity = {1.f, 0.f, 0.f}, .q = -1.f}};

    rotate_magnetic(charges, FieldParams{.softening = 0.f, .b_external = 1.f}, 0.1f);

    EXPECT_GT(charges[0].velocity.y, 0.f);
    EXPECT_NEAR(glm::length(charges[0].velocity), 1.f, 1e-5f);
}

TEST(MagnetismTest, FixedAndMasslessChargesStayPut) {
    std::array charges{
            Charge{.velocity = {1.f, 0.f, 0.f}, .q = 1.f, .fixed = true},
            Charge{.velocity = {1.f, 0.f, 0.f}, .q = 1.f, .mass = 0.f},
    };
    const std::array before = charges;

    rotate_magnetic(charges, FieldParams{.b_external = 3.f}, 0.2f);

    EXPECT_EQ(charges[0].velocity, before[0].velocity);
    EXPECT_EQ(charges[1].velocity, before[1].velocity);
}

TEST(MagnetismTest, DisabledFlagIgnoresMagneticField) {
    const Charge moving = moving_right();
    const Charge neighbour{.position = {0.f, 2.f, 0.f}, .velocity = {1.f, 0.f, 0.f}, .q = 1.f};
    Simulation magnetic_off(FieldParams{.k = 0.f, .softening = 0.f, .b_external = 2.f});
    Simulation electric(FieldParams{.k = 0.f, .softening = 0.f});
    magnetic_off.charges().push_back(moving);
    magnetic_off.charges().push_back(neighbour);
    electric.charges().push_back(moving);
    electric.charges().push_back(neighbour);

    for (int i = 0; i < 40; ++i) {
        magnetic_off.step(1.f / 60.f);
        electric.step(1.f / 60.f);
    }

    for (std::size_t i = 0; i < 2; ++i) {
        EXPECT_EQ(magnetic_off.charges()[i].position, electric.charges()[i].position);
        EXPECT_EQ(magnetic_off.charges()[i].velocity, electric.charges()[i].velocity);
    }
}

TEST(MagnetismTest, EnabledFlagDeflectsTrajectory) {
    const FieldParams params{.k = 0.f, .softening = 0.f, .b_external = 1.f};
    const DynamicsOptions on_dynamics{.collisions = false, .magnetic = true};
    Simulation off(params, DynamicsOptions{.collisions = false});
    Simulation on(params, on_dynamics);
    off.charges().push_back(moving_right());
    on.charges().push_back(moving_right());

    for (int i = 0; i < 30; ++i) {
        off.step(1.f / 60.f);
        on.step(1.f / 60.f);
    }

    EXPECT_NEAR(off.charges()[0].velocity.y, 0.f, 1e-5f);
    EXPECT_LT(on.charges()[0].velocity.y, 0.f);
    EXPECT_LT(on.charges()[0].position.y, off.charges()[0].position.y);
}

// Two like charges moving side by side: electrically they repel, magnetically (parallel currents) they
// attract. With c = 1 the magnetic pull is as strong as it ever gets, so it must visibly reduce the
// repulsion compared with the same run without the Lorentz force.
TEST(MagnetismTest, ParallelMovingChargesAttractMagnetically) {
    const FieldParams params{.k = 1.f, .softening = 0.f, .light_speed = 1.f};
    Simulation off(params, DynamicsOptions{.collisions = false});
    Simulation on(params, DynamicsOptions{.collisions = false, .magnetic = true});
    for (Simulation* sim : {&off, &on}) {
        sim->charges().push_back(Charge{.velocity = {0.5f, 0.f, 0.f}, .q = 1.f});
        sim->charges().push_back(Charge{.position = {0.f, 2.f, 0.f}, .velocity = {0.5f, 0.f, 0.f}, .q = 1.f});
    }

    on.step(0.05f);
    off.step(0.05f);

    // Charge 0 sits below charge 1: repulsion pushes it down, the magnetic pull lifts it back up.
    EXPECT_GT(on.charges()[0].velocity.y, off.charges()[0].velocity.y);
    EXPECT_LT(on.charges()[1].velocity.y, off.charges()[1].velocity.y);
}

TEST(MagnetismTest, CouplingFallsWithSquareOfLightSpeed) {
    const std::array charges{moving_right()};
    const glm::vec3 above{0.f, 2.f, 0.f};

    const float slow_light = magnetic_z(charges, above, FieldParams{.k = 1.f, .softening = 0.f, .light_speed = 1.f});
    const float fast_light = magnetic_z(charges, above, FieldParams{.k = 1.f, .softening = 0.f, .light_speed = 2.f});

    EXPECT_NEAR(fast_light, slow_light / 4.f, 1e-6f);
}

// Uniform B only: radius m v / (q B) and period 2π m / (q B), independent of the speed.
TEST(MagnetismTest, CyclotronRadiusAndPeriod) {
    constexpr float kB = 2.f;
    constexpr float kQ = 0.5f;
    constexpr float kV = 1.f;
    const float radius = kV / (kQ * kB);
    const float period = 2.f * 3.14159265f / (kQ * kB);
    constexpr float dt = 1.f / 600.f;
    Simulation sim(FieldParams{.k = 0.f, .softening = 0.f, .b_external = kB},
            DynamicsOptions{.collisions = false, .magnetic = true});
    sim.charges().push_back(Charge{.velocity = {kV, 0.f, 0.f}, .q = kQ});

    float farthest = 0.f;
    const int steps = static_cast<int>(std::lround(period / dt));
    for (int i = 0; i < steps; ++i) {
        sim.step(dt);
        farthest = std::max(farthest, glm::length(sim.charges()[0].position));
    }

    EXPECT_NEAR(farthest, 2.f * radius, 1e-2f) << "diameter";
    EXPECT_NEAR(glm::length(sim.charges()[0].position), 0.f, 1e-2f) << "back at the start after one period";
}

// The regression this file was missing: a single full rotation before the drift let K + U creep by
// percents per minute once E and B acted together. Magnetic forces do no work, so K + U must hold.
TEST(MagnetismTest, ElectricAndMagneticTogetherConserveEnergy) {
    Simulation sim(FieldParams{.k = 1.f, .softening = 0.25f, .light_speed = 20.f, .b_external = 0.5f},
            DynamicsOptions{.collisions = false, .magnetic = true});
    sim.charges().push_back(Charge{.q = 2.f, .fixed = true});
    sim.charges().push_back(Charge{.position = {0.f, 3.f, 0.f}, .velocity = {0.577f, 0.f, 0.f}, .q = -0.5f});
    const float e0 = sim.total_energy();

    for (int i = 0; i < 60 * 120; ++i) {
        sim.step(1.f / 60.f);
    }

    EXPECT_NEAR(sim.total_energy(), e0, 1e-3f * std::abs(e0));
}

TEST(MagnetismTest, SpeedStaysBelowLightSpeed) {
    Simulation sim(FieldParams{.k = 0.f, .light_speed = 2.f}, DynamicsOptions{.collisions = false});
    sim.charges().push_back(Charge{.velocity = {5.f, 0.f, 0.f}, .q = 1.f});

    sim.step(1.f / 60.f);

    EXPECT_LE(glm::length(sim.charges()[0].velocity), kMaxLightFraction * 2.f + 1e-5f);
}

TEST(MagnetismTest, ExternalFieldPreservesKineticEnergy) {
    Simulation sim(FieldParams{.k = 0.f, .softening = 0.f, .b_external = 1.f},
            DynamicsOptions{.collisions = false, .magnetic = true});
    sim.charges().push_back(moving_right());
    const float kinetic0 = kinetic_energy(sim.charges());

    for (int i = 0; i < 1200; ++i) {
        sim.step(1.f / 60.f);
    }

    EXPECT_NEAR(kinetic_energy(sim.charges()), kinetic0, 1e-4f);
    EXPECT_EQ(sim.charges()[0].position.z, 0.f);
    EXPECT_EQ(sim.charges()[0].velocity.z, 0.f);
    EXPECT_LT(sim.charges()[0].velocity.y, 0.f);
}

TEST(MagnetismTest, CoilFieldProfile) {
    const Coil coil{.centre = {1.f, -2.f, 0.f}, .radius = 5.f, .centre_field = 2.f};

    EXPECT_NEAR(coil_field_z(coil, coil.centre), 2.f, 1e-4f);
    // Inside it grows towards the wire, outside it has the opposite sign and dies away.
    EXPECT_GT(coil_field_z(coil, coil.centre + glm::vec3{3.f, 0.f, 0.f}), 2.f);
    EXPECT_LT(coil_field_z(coil, coil.centre + glm::vec3{0.f, 7.f, 0.f}), 0.f);
    EXPECT_NEAR(coil_field_z(coil, coil.centre + glm::vec3{40.f, 0.f, 0.f}), 0.f, 1e-2f);
    // Rotationally symmetric.
    EXPECT_NEAR(coil_field_z(coil, coil.centre + glm::vec3{0.f, 3.f, 0.f}),
            coil_field_z(coil, coil.centre + glm::vec3{-3.f, 0.f, 0.f}), 1e-3f);
    // Reversing the current reverses the field.
    const Coil reversed{.centre = coil.centre, .radius = 5.f, .centre_field = -2.f};
    EXPECT_NEAR(coil_field_z(reversed, coil.centre + glm::vec3{2.f, 1.f, 0.f}),
            -coil_field_z(coil, coil.centre + glm::vec3{2.f, 1.f, 0.f}), 1e-4f);
}

TEST(MagnetismTest, CoilsAddToTheTotalFieldAndIgnoreLightSpeed) {
    const std::array coils{Coil{}};
    const glm::vec3 p{2.f, 1.f, 0.f};

    const float slow = magnetic_z({}, p, FieldParams{.light_speed = 3.f, .b_external = 0.5f}, kNoCharge, coils);
    const float fast = magnetic_z({}, p, FieldParams{.light_speed = 50.f, .b_external = 0.5f}, kNoCharge, coils);

    EXPECT_NEAR(slow, 0.5f + coil_field_z(coils[0], p), 1e-5f);
    EXPECT_NEAR(slow, fast, 1e-5f);
}

}
}
