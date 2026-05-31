#include <game/field_viz.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

#include <array>
#include <cmath>

namespace game {
namespace {

constexpr FieldParams kParams{.k = 1.f, .softening = 0.25f};

FieldLineOptions small_bounds() {
    FieldLineOptions options;
    options.bounds = Bounds{{-20.f, -20.f}, {20.f, 20.f}};
    return options;
}

TEST(FieldVizTest, LoneChargeLinesAreRadialAndEscape) {
    const std::array charges{Charge{.position = {1.f, -1.f, 0.f}, .q = 1.f}};

    const std::vector<FieldLine> lines = trace_field_lines(charges, kParams, small_bounds());

    ASSERT_EQ(lines.size(), 8u);
    for (const FieldLine& line : lines) {
        const glm::vec3 first = glm::normalize(line[1] - charges[0].position);
        const glm::vec3 last = glm::normalize(line.back() - charges[0].position);
        EXPECT_NEAR(glm::dot(first, last), 1.f, 1e-3f);
        EXPECT_GT(glm::length(line.back() - charges[0].position), 19.f);
    }
}

TEST(FieldVizTest, LineCountIsProportionalToCharge) {
    const std::array weak{Charge{.q = 1.f}};
    const std::array strong{Charge{.q = 3.f}};

    EXPECT_EQ(trace_field_lines(weak, kParams, small_bounds()).size(), 8u);
    EXPECT_EQ(trace_field_lines(strong, kParams, small_bounds()).size(), 24u);
}

TEST(FieldVizTest, LoneNegativeChargeStillGetsLines) {
    const std::array charges{Charge{.q = -1.f}};

    const std::vector<FieldLine> lines = trace_field_lines(charges, kParams, small_bounds());

    EXPECT_EQ(lines.size(), 8u);
}

TEST(FieldVizTest, DipoleLinesRunFromPlusToMinusWithoutDuplicates) {
    const std::array charges{
            Charge{.position = {-2.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {2.f, 0.f, 0.f}, .q = -1.f},
    };

    // Wide bounds so even the big loops leaving the back of + close on -.
    FieldLineOptions options;
    options.bounds = Bounds{{-200.f, -200.f}, {200.f, 200.f}};
    const std::vector<FieldLine> lines = trace_field_lines(charges, kParams, options);

    int ending_on_minus = 0;
    for (const FieldLine& line : lines) {
        EXPECT_LT(glm::length(line.front() - charges[0].position), 0.31f) << "every line starts at +";
        if (glm::length(line.back() - charges[1].position) < 1e-5f) {
            ++ending_on_minus;
        }
    }
    // Equal and opposite charges: every line closes on −. Backward lines from − land on + and are dropped.
    EXPECT_EQ(lines.size(), 8u);
    EXPECT_EQ(ending_on_minus, 8);
}

TEST(FieldVizTest, LinesFollowFieldDirection) {
    const std::array charges{
            Charge{.position = {-2.f, 1.f, 0.f}, .q = 2.f},
            Charge{.position = {3.f, -1.f, 0.f}, .q = -1.f},
    };

    for (const FieldLine& line : trace_field_lines(charges, kParams, small_bounds())) {
        for (std::size_t i = 1; i + 2 < line.size(); i += 7) {
            const glm::vec3 step = line[i + 1] - line[i];
            const glm::vec3 e = field_at(charges, line[i], kParams);
            EXPECT_GT(glm::dot(glm::normalize(step), glm::normalize(e)), 0.95f);
        }
    }
}

TEST(FieldVizTest, GridIsWorldAlignedAndMatchesField) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f}};
    const Bounds bounds{{-3.3f, -2.1f}, {3.3f, 2.1f}};

    const std::vector<FieldSample> samples = sample_field_grid(charges, kParams, bounds, 1.f);

    EXPECT_FALSE(samples.empty());
    for (const FieldSample& s : samples) {
        EXPECT_FLOAT_EQ(s.position.x, std::round(s.position.x));
        EXPECT_FLOAT_EQ(s.position.y, std::round(s.position.y));
        EXPECT_TRUE(bounds.contains(s.position));
        EXPECT_GT(glm::length(s.position), 0.5f) << "points inside the charge are skipped";
        EXPECT_NEAR(glm::length(s.field - field_at(charges, s.position, kParams)), 0.f, 1e-6f);
    }
}

TEST(FieldVizTest, StrengthScaleIsMonotonicAndBounded) {
    EXPECT_FLOAT_EQ(field_strength01(0.f), 0.f);
    EXPECT_LT(field_strength01(0.1f), field_strength01(1.f));
    EXPECT_LT(field_strength01(1.f), field_strength01(10.f));
    EXPECT_FLOAT_EQ(field_strength01(1e6f), 1.f);
}

TEST(FieldVizTest, FlowParticlesMoveAlongField) {
    const std::array charges{Charge{.position = {0.f, 0.f, 0.f}, .q = 1.f}};
    const Bounds bounds{{-10.f, -10.f}, {10.f, 10.f}};
    FlowField flow(200, 7);

    flow.update(charges, kParams, bounds, 0.f);
    std::vector<FlowParticle> before = flow.particles();
    flow.update(charges, kParams, bounds, 0.01f);

    int checked = 0;
    for (std::size_t i = 0; i < before.size(); ++i) {
        const FlowParticle& a = before[i];
        const FlowParticle& b = flow.particles()[i];
        if (b.age < a.age) {
            continue;  // respawned this step
        }
        const glm::vec3 step = b.position - a.position;
        ASSERT_GT(glm::length(step), 0.f);
        EXPECT_GT(glm::dot(glm::normalize(step), glm::normalize(a.position)), 0.99f) << "moves away from +";
        ++checked;
    }
    EXPECT_GT(checked, 150);
}

TEST(FieldVizTest, FlowParticlesAreBornAtPositiveSources) {
    const std::array charges{Charge{.position = {3.f, 2.f, 0.f}, .q = 1.f}};
    const Bounds bounds{{-10.f, -10.f}, {10.f, 10.f}};
    FlowField flow(400, 11);

    flow.update(charges, kParams, bounds, 0.f);

    int near_source = 0;
    for (const FlowParticle& p : flow.particles()) {
        if (glm::length(p.position - charges[0].position) < 1.f) {
            ++near_source;
        }
    }
    // About a third are seeded near a + charge; uniform spawning would put ~3 of 400 there.
    EXPECT_GT(near_source, 80);
}

TEST(FieldVizTest, FlowParticlesStayInBoundsAndOutOfCharges) {
    const std::array charges{
            Charge{.position = {-2.f, 0.f, 0.f}, .q = 1.f},
            Charge{.position = {2.f, 0.f, 0.f}, .q = -1.f},
    };
    const Bounds bounds{{-5.f, -4.f}, {5.f, 4.f}};
    FlowField flow(300, 3);

    for (int i = 0; i < 300; ++i) {
        flow.update(charges, kParams, bounds, 1.f / 60.f);
    }

    for (const FlowParticle& p : flow.particles()) {
        EXPECT_TRUE(bounds.contains(p.position));
        EXPECT_GT(glm::length(p.position - charges[1].position), flow.capture_radius);
        EXPECT_EQ(p.position.z, 0.f);
    }
}

}
}
