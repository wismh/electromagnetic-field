#include <game/electrostatics.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <array>
#include <cmath>

namespace game {
namespace {

constexpr FieldParams kExact{.k = 1.f, .softening = 0.f};

TEST(ElectrostaticsTest, SingleChargeFieldMagnitudeIsKqOverRSquared) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = 2.f}};
    const FieldParams params{.k = 3.f, .softening = 0.f};

    const glm::vec3 e = field_at(charges, {0.f, 2.f, 0.f}, params);

    EXPECT_NEAR(glm::length(e), 3.f * 2.f / 4.f, 1e-6f);
}

TEST(ElectrostaticsTest, PositiveChargeFieldPointsAway) {
    const std::array charges{Charge{.position = {1.f, 1.f, 0.f}, .q = 1.f}};

    const glm::vec3 e = field_at(charges, {4.f, 5.f, 0.f}, kExact);

    const glm::vec3 expected_dir = glm::normalize(glm::vec3{3.f, 4.f, 0.f});
    EXPECT_NEAR(glm::dot(glm::normalize(e), expected_dir), 1.f, 1e-6f);
}

TEST(ElectrostaticsTest, NegativeChargeFieldPointsToward) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = -1.f}};

    const glm::vec3 e = field_at(charges, {2.f, 0.f, 0.f}, kExact);

    EXPECT_LT(e.x, 0.f);
    EXPECT_NEAR(e.y, 0.f, 1e-7f);
}

TEST(ElectrostaticsTest, FieldVanishesMidwayBetweenEqualCharges) {
    const std::array charges{
            Charge{.position = {-1.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {1.f, 0.f, 0.f}, .q = 1.f},
    };

    const glm::vec3 e = field_at(charges, {0.f, 0.f, 0.f}, kExact);

    EXPECT_NEAR(glm::length(e), 0.f, 1e-6f);
}

TEST(ElectrostaticsTest, SuperpositionAddsFieldsOfEachCharge) {
    const Charge a{.position = {-1.f, 0.5f, 0.f}, .q = 1.5f};
    const Charge b{.position = {2.f, -1.f, 0.f}, .q = -0.7f};
    const std::array only_a{a};
    const std::array only_b{b};
    const std::array both{a, b};
    const glm::vec3 p{0.3f, 1.2f, 0.f};

    const glm::vec3 sum = field_at(only_a, p, kExact) + field_at(only_b, p, kExact);
    const glm::vec3 e = field_at(both, p, kExact);

    EXPECT_NEAR(glm::length(e - sum), 0.f, 1e-6f);
}

TEST(ElectrostaticsTest, PotentialIsKqOverR) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = -2.f}};

    EXPECT_NEAR(potential_at(charges, {0.f, 4.f, 0.f}, kExact), -0.5f, 1e-6f);
}

TEST(ElectrostaticsTest, FieldIsMinusGradientOfPotential) {
    const std::array charges{
            Charge{.position = {-1.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {1.f, 0.5f, 0.f}, .q = -2.f},
    };
    const FieldParams params{.k = 1.f, .softening = 0.1f};
    const glm::vec3 p{0.2f, -0.6f, 0.f};
    constexpr float h = 1e-3f;
    const glm::vec3 dx{h, 0.f, 0.f};
    const glm::vec3 dy{0.f, h, 0.f};

    const glm::vec3 e = field_at(charges, p, params);
    const float dphi_dx = (potential_at(charges, p + dx, params) - potential_at(charges, p - dx, params)) / (2.f * h);
    const float dphi_dy = (potential_at(charges, p + dy, params) - potential_at(charges, p - dy, params)) / (2.f * h);

    EXPECT_NEAR(e.x, -dphi_dx, 1e-2f * glm::length(e));
    EXPECT_NEAR(e.y, -dphi_dy, 1e-2f * glm::length(e));
}

TEST(ElectrostaticsTest, SofteningKeepsFieldFiniteAtCharge) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f}};

    const glm::vec3 e = field_at(charges, {0.f, 0.f, 0.f}, FieldParams{.k = 1.f, .softening = 0.1f});

    EXPECT_TRUE(std::isfinite(e.x) && std::isfinite(e.y) && std::isfinite(e.z));
}

TEST(ElectrostaticsTest, ChargeIgnoresItsOwnField) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {2.f, 0.f, 0.f}, .q = 3.f},
    };

    const glm::vec3 f = force_on(charges, 0, kExact);

    EXPECT_NEAR(f.x, -1.f * 3.f / 4.f, 1e-6f);
    EXPECT_NEAR(f.y, 0.f, 1e-7f);
}

TEST(ElectrostaticsTest, ForceIsChargeTimesField) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = -1.5f},
            Charge{.position = {1.f, 2.f, 0.f}, .q = 0.5f},
            Charge{.position = {-2.f, 1.f, 0.f}, .q = 2.f},
    };

    const glm::vec3 f = force_on(charges, 0, kExact);
    const glm::vec3 e = field_at(charges, charges[0].position, kExact, 0);

    EXPECT_NEAR(glm::length(f - charges[0].q * e), 0.f, 1e-6f);
}

TEST(ElectrostaticsTest, NewtonsThirdLaw) {
    const std::array charges{
            Charge{.position = {0.3f, -0.4f, 0.f}, .q = 1.2f},
            Charge{.position = {-1.f, 2.f, 0.f}, .q = -0.8f},
    };

    const glm::vec3 f01 = force_on(charges, 0, kExact);
    const glm::vec3 f10 = force_on(charges, 1, kExact);

    EXPECT_NEAR(glm::length(f01 + f10), 0.f, 1e-6f);
}

TEST(ElectrostaticsTest, OppositeChargesAttractLikeChargesRepel) {
    const std::array opposite{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {1.f, 0.f, 0.f}, .q = -1.f},
    };
    const std::array like{
            Charge{.position = {0.f, 0.f, 0.f}, .q = -1.f},
            Charge{.position = {1.f, 0.f, 0.f}, .q = -1.f},
    };

    EXPECT_GT(force_on(opposite, 0, kExact).x, 0.f);
    EXPECT_LT(force_on(like, 0, kExact).x, 0.f);
}

TEST(ElectrostaticsTest, PlanarChargesGiveZeroZ) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {1.f, 3.f, 0.f}, .q = -2.f},
    };

    EXPECT_EQ(field_at(charges, {0.5f, -0.7f, 0.f}, kExact).z, 0.f);
    EXPECT_EQ(force_on(charges, 1, kExact).z, 0.f);
}

TEST(ElectrostaticsTest, PotentialEnergyOfPair) {
    const std::array charges{
            Charge{.position = {0.f, 0.f, 0.f}, .q = 2.f},
            Charge{.position = {0.f, 4.f, 0.f}, .q = -3.f},
    };

    EXPECT_NEAR(potential_energy(charges, kExact), -1.5f, 1e-6f);
}

}
}
