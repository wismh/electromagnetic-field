#include <game/simulation.h>
#include <game/trails.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

namespace game {
namespace {

// k = 0 switches the field off so only the collision response is under test.
constexpr FieldParams kNoField{.k = 0.f, .softening = 0.25f};

Simulation head_on(float restitution) {
    Simulation sim(kNoField, DynamicsOptions{.collisions = true, .restitution = restitution, .max_speed = 100.f});
    sim.charges().push_back(Charge{.position = {-1.f, 0.f, 0.f}, .velocity = {1.f, 0.f, 0.f}, .q = 1.f});
    sim.charges().push_back(Charge{.position = {1.f, 0.f, 0.f}, .velocity = {-1.f, 0.f, 0.f}, .q = 1.f});
    return sim;
}

void run(Simulation& sim, float seconds, float dt = 1e-3f) {
    for (float t = 0.f; t < seconds; t += dt) {
        sim.step(dt);
    }
}

TEST(DynamicsTest, ElasticHeadOnCollisionSwapsVelocities) {
    Simulation sim = head_on(1.f);

    run(sim, 2.f);

    EXPECT_NEAR(sim.charges()[0].velocity.x, -1.f, 1e-4f);
    EXPECT_NEAR(sim.charges()[1].velocity.x, 1.f, 1e-4f);
}

TEST(DynamicsTest, RestitutionScalesSeparationSpeed) {
    Simulation sim = head_on(0.5f);

    run(sim, 2.f);

    const float separation = sim.charges()[1].velocity.x - sim.charges()[0].velocity.x;
    EXPECT_NEAR(separation, 0.5f * 2.f, 1e-4f);
    EXPECT_NEAR(sim.charges()[0].velocity.x + sim.charges()[1].velocity.x, 0.f, 1e-5f) << "momentum";
}

TEST(DynamicsTest, FreeChargeBouncesOffFixedOne) {
    Simulation sim(kNoField, DynamicsOptions{.collisions = true, .restitution = 1.f, .max_speed = 100.f});
    sim.charges().push_back(Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f, .fixed = true});
    sim.charges().push_back(Charge{.position = {2.f, 0.f, 0.f}, .velocity = {-1.f, 0.f, 0.f}, .q = 1.f});

    run(sim, 3.f);

    EXPECT_EQ(sim.charges()[0].position, glm::vec3(0.f));
    EXPECT_NEAR(sim.charges()[1].velocity.x, 1.f, 1e-4f);
}

TEST(DynamicsTest, OppositeChargesDoNotPassThroughEachOther) {
    Simulation sim;
    sim.charges().push_back(Charge{.position = {-2.f, 0.f, 0.f}, .q = 1.f});
    sim.charges().push_back(Charge{.position = {2.f, 0.f, 0.f}, .q = -1.f});
    const float contact = 2.f * charge_radius(1.f);

    for (int i = 0; i < 5000; ++i) {
        sim.step(1e-3f);
        ASSERT_LT(sim.charges()[0].position.x, sim.charges()[1].position.x) << "step " << i;
        ASSERT_GT(glm::length(sim.charges()[1].position - sim.charges()[0].position), 0.98f * contact);
    }
}

TEST(DynamicsTest, CollisionsCanBeDisabled) {
    Simulation sim(kNoField, DynamicsOptions{.collisions = false, .restitution = 1.f, .max_speed = 100.f});
    sim.charges().push_back(Charge{.position = {-1.f, 0.f, 0.f}, .velocity = {1.f, 0.f, 0.f}, .q = 1.f});
    sim.charges().push_back(Charge{.position = {1.f, 0.f, 0.f}, .velocity = {-1.f, 0.f, 0.f}, .q = 1.f});

    run(sim, 2.f);

    EXPECT_GT(sim.charges()[0].position.x, 0.5f) << "passed through";
}

TEST(DynamicsTest, SpeedIsCapped) {
    Simulation sim(kNoField, DynamicsOptions{.collisions = true, .restitution = 1.f, .max_speed = 5.f});
    sim.charges().push_back(Charge{.velocity = {30.f, 40.f, 0.f}, .q = 1.f});

    sim.step(1e-3f);

    EXPECT_NEAR(glm::length(sim.charges()[0].velocity), 5.f, 1e-4f);
    EXPECT_NEAR(sim.charges()[0].velocity.y / sim.charges()[0].velocity.x, 4.f / 3.f, 1e-4f) << "direction kept";
}

TEST(TrailsTest, RecordsMovingChargesWithMinimumSpacing) {
    Trails trails;
    trails.min_spacing = 0.1f;
    Charge c{.q = 1.f, .id = 7};

    for (int i = 0; i <= 10; ++i) {
        c.position = {0.03f * static_cast<float>(i), 0.f, 0.f};
        trails.record(std::span(&c, 1), 0.1f * static_cast<float>(i));
    }

    ASSERT_EQ(trails.trails().size(), 1u);
    const Trail& t = trails.trails()[0];
    EXPECT_EQ(t.id, 7u);
    // Positions 0, 0.12, 0.24 (each >= 0.1 from the previous kept point).
    EXPECT_EQ(t.points.size(), 3u);
}

TEST(TrailsTest, OldPointsExpire) {
    Trails trails;
    trails.duration = 1.f;
    Charge c{.q = 1.f, .id = 1};

    for (int i = 0; i < 30; ++i) {
        c.position = {static_cast<float>(i) * 0.5f, 0.f, 0.f};
        trails.record(std::span(&c, 1), static_cast<float>(i) * 0.1f);
    }

    for (const TrailPoint& p : trails.trails()[0].points) {
        EXPECT_GE(p.time, 2.9f - 1.f);
    }
}

TEST(TrailsTest, TeleportRestartsTrail) {
    Trails trails;
    Charge c{.q = 1.f, .id = 1};
    trails.record(std::span(&c, 1), 0.f);
    c.position = {0.5f, 0.f, 0.f};
    trails.record(std::span(&c, 1), 0.1f);

    c.position = {10.f, 0.f, 0.f};
    trails.record(std::span(&c, 1), 0.2f);

    ASSERT_EQ(trails.trails()[0].points.size(), 1u);
    EXPECT_EQ(trails.trails()[0].points[0].position, glm::vec3(10.f, 0.f, 0.f));
}

TEST(TrailsTest, RemovedChargesAndFixedNewcomersHaveNoTrail) {
    Trails trails;
    std::vector<Charge> charges{
            Charge{.q = 1.f, .id = 1},
            Charge{.q = -1.f, .fixed = true, .id = 2},
    };
    trails.record(charges, 0.f);
    ASSERT_EQ(trails.trails().size(), 1u);
    EXPECT_EQ(trails.trails()[0].id, 1u);

    charges.erase(charges.begin());
    trails.record(charges, 0.1f);

    EXPECT_TRUE(trails.trails().empty());
}

}
}
