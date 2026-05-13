#include <game/scene.h>

#include <game/electrostatics.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <array>

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
                    Preset::Rutherford, Preset::Swarm}) {
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

}
}
