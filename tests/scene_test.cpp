#include <game/charge_limit.h>
#include <game/scene.h>

#include <game/electrostatics.h>
#include <game/simulation.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <array>
#include <cmath>

namespace game {
namespace {

float net_charge(const std::vector<Charge>& charges) {
    float total = 0.f;
    for (const Charge& c : charges) {
        total += c.q;
    }
    return total;
}

TEST(SceneTest, NeutralPresetsHaveZeroNetCharge) {
    EXPECT_FLOAT_EQ(net_charge(make_preset(Preset::Dipole)), 0.f);
    EXPECT_FLOAT_EQ(net_charge(make_preset(Preset::Quadrupole)), 0.f);
    EXPECT_FLOAT_EQ(net_charge(make_preset(Preset::Capacitor)), 0.f);
}

TEST(SceneTest, PresetsFitShaderLimitAndStayInPlane) {
    for (const Preset preset :
            {Preset::Dipole, Preset::LikePair, Preset::Quadrupole, Preset::Capacitor, Preset::Orbit,
                    Preset::Rutherford, Preset::Swarm, Preset::Cyclotron, Preset::ExBDrift, Preset::Coil}) {
        const std::vector<Charge> charges = make_preset(preset);
        EXPECT_FALSE(charges.empty());
        EXPECT_LE(charges.size(), kMaxCharges);
        for (const Charge& c : charges) {
            EXPECT_EQ(c.position.z, 0.f);
            EXPECT_EQ(c.velocity.z, 0.f);
        }
    }
}

TEST(SceneTest, OrbitPresetSpeedBalancesCoulombAttraction) {
    const std::vector<Charge> charges = make_preset(Preset::Orbit);
    ASSERT_EQ(charges.size(), 2u);
    const Charge& satellite = charges[1];
    const float r = glm::length(satellite.position - charges[0].position);

    const glm::vec3 f = force_on(charges, 1, FieldParams{.k = 1.f, .softening = 0.f});
    const float centripetal = satellite.mass * glm::dot(satellite.velocity, satellite.velocity) / r;

    EXPECT_NEAR(glm::length(f), centripetal, 1e-5f);
}

TEST(SceneTest, RutherfordProjectilesAreRepelledByFixedNucleus) {
    const std::vector<Charge> charges = make_preset(Preset::Rutherford);
    ASSERT_GT(charges.size(), 2u);
    EXPECT_TRUE(charges[0].fixed);
    for (std::size_t i = 1; i < charges.size(); ++i) {
        EXPECT_FALSE(charges[i].fixed);
        EXPECT_GT(charges[i].q * charges[0].q, 0.f) << "same sign as the nucleus";
        EXPECT_GT(charges[i].velocity.x, 0.f) << "flying towards it";
    }
}

TEST(SceneTest, SwarmIsNeutralAndFree) {
    const std::vector<Charge> charges = make_preset(Preset::Swarm);
    EXPECT_FLOAT_EQ(net_charge(charges), 0.f);
    for (const Charge& c : charges) {
        EXPECT_FALSE(c.fixed);
    }
}

TEST(SceneTest, PickFindsChargeUnderPoint) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {5.f, 0.f, 0.f}, .q = -1.f},
    };

    EXPECT_EQ(pick_charge(charges, {5.1f, 0.1f, 0.f}), std::optional<std::size_t>{1});
    EXPECT_EQ(pick_charge(charges, {2.5f, 0.f, 0.f}), std::nullopt);
}

TEST(SceneTest, PickPrefersNearestOverlappingCharge) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {0.3f, 0.f, 0.f}, .q = 1.f},
    };

    EXPECT_EQ(pick_charge(charges, {0.25f, 0.f, 0.f}), std::optional<std::size_t>{1});
}

TEST(SceneTest, BiggerChargeIsDrawnBigger) {
    EXPECT_GT(charge_radius(4.f), charge_radius(1.f));
    EXPECT_FLOAT_EQ(charge_radius(-2.f), charge_radius(2.f));
}

TEST(SceneTest, StepChargeKeepsSignAndClamps) {
    EXPECT_FLOAT_EQ(step_charge_magnitude(1.f, 1.f), 1.25f);
    EXPECT_FLOAT_EQ(step_charge_magnitude(-1.f, 1.f), -1.25f);
    EXPECT_FLOAT_EQ(step_charge_magnitude(-1.f, -2.f), -0.5f);
    EXPECT_FLOAT_EQ(step_charge_magnitude(0.25f, -1.f), kMinAbsCharge);
    EXPECT_FLOAT_EQ(step_charge_magnitude(-4.9f, 3.f), -kMaxAbsCharge);
}

TEST(SceneTest, OnlyMagneticScenesTurnMagnetismOn) {
    EXPECT_FALSE(scene_settings(Preset::Dipole).magnetic);
    EXPECT_EQ(scene_settings(Preset::Orbit).b_external, 0.f);
    EXPECT_TRUE(scene_settings(Preset::Cyclotron).magnetic);
    EXPECT_GT(scene_settings(Preset::Cyclotron).b_external, 0.f);
    EXPECT_TRUE(scene_settings(Preset::ExBDrift).magnetic);
}

Simulation magnetic_scene(Preset preset) {
    const SceneSettings settings = scene_settings(preset);
    Simulation sim(FieldParams{.b_external = settings.b_external},
            DynamicsOptions{.collisions = false, .magnetic = settings.magnetic});
    sim.charges() = make_preset(preset);
    sim.coils() = settings.coils;
    return sim;
}

// Same q/m, so after one cyclotron period every charge is back where it started.
TEST(SceneTest, CyclotronChargesShareOnePeriod) {
    Simulation sim = magnetic_scene(Preset::Cyclotron);
    const std::vector<Charge> start = sim.charges();
    const Charge& first = start.front();
    const float period = 2.f * 3.14159265f * first.mass / (std::abs(first.q) * kCyclotronField);
    constexpr float dt = 1.f / 600.f;

    for (int i = 0; i < static_cast<int>(std::lround(period / dt)); ++i) {
        sim.step(dt);
    }

    for (std::size_t i = 0; i < start.size(); ++i) {
        EXPECT_NEAR(glm::length(sim.charges()[i].position - start[i].position), 0.f, 0.15f) << "charge " << i;
    }
}

// Both signs drift the same way (−x for E down, B out of the page).
TEST(SceneTest, ExBDriftMovesBothSignsSidewaysBetweenThePlates) {
    Simulation sim = magnetic_scene(Preset::ExBDrift);
    std::vector<std::size_t> free_charges;
    for (std::size_t i = 0; i < sim.charges().size(); ++i) {
        if (!sim.charges()[i].fixed) {
            free_charges.push_back(i);
        }
    }
    ASSERT_EQ(free_charges.size(), 3u);
    std::vector<float> start_x;
    for (std::size_t i : free_charges) {
        start_x.push_back(sim.charges()[i].position.x);
    }

    for (int step = 0; step < 60 * 15; ++step) {
        sim.step(1.f / 60.f);
        for (std::size_t i : free_charges) {
            ASSERT_LT(std::abs(sim.charges()[i].position.y), 2.5f) << "charge " << i << " step " << step;
        }
    }

    for (std::size_t k = 0; k < free_charges.size(); ++k) {
        EXPECT_LT(sim.charges()[free_charges[k]].position.x, start_x[k] - 3.f) << "charge " << free_charges[k];
    }
}

// Grad-B: + and − travel around the coil in opposite directions. The coil field does no work.
TEST(SceneTest, CoilChargesDriftAroundTheCoilInOppositeDirections) {
    Simulation sim = magnetic_scene(Preset::Coil);
    ASSERT_EQ(sim.coils().size(), 1u);
    const float wire = sim.coils()[0].radius;
    const std::vector<Charge> start = sim.charges();
    const float energy0 = sim.total_energy();
    std::vector<float> azimuth(start.size(), 0.f);
    std::vector<float> previous;
    for (const Charge& c : start) {
        previous.push_back(std::atan2(c.position.y, c.position.x));
    }

    for (int step = 0; step < 60 * 40; ++step) {
        sim.step(1.f / 60.f);
        for (std::size_t i = 0; i < start.size(); ++i) {
            const Charge& c = sim.charges()[i];
            ASSERT_GT(std::abs(glm::length(c.position) - wire), 0.5f) << "charge " << i << " reached the wire";
            const float angle = std::atan2(c.position.y, c.position.x);
            float delta = angle - previous[i];
            delta = std::remainder(delta, 2.f * 3.14159265f);
            azimuth[i] += delta;
            previous[i] = angle;
        }
    }

    for (std::size_t i = 0; i < start.size(); ++i) {
        const float travelled = std::abs(azimuth[i]) * 180.f / 3.14159265f;
        EXPECT_GT(travelled, 25.f) << "charge " << i << " drifted only " << travelled << " degrees";
    }
    EXPECT_NEAR(sim.total_energy(), energy0, 1e-3f * std::abs(energy0));
    // Opposite charge, opposite drift.
    EXPECT_LT(azimuth[0] * azimuth[1], 0.f);
    EXPECT_LT(azimuth[2] * azimuth[3], 0.f);
}

}
}
