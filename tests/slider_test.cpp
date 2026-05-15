#include <game/slider.h>

#include <gtest/gtest.h>

namespace game {
namespace {

TEST(SliderTest, MapsEndsAndMiddle) {
    const SliderRange range{.min = 0.1f, .max = 5.1f};

    EXPECT_FLOAT_EQ(range.from_fraction(0.f), 0.1f);
    EXPECT_FLOAT_EQ(range.from_fraction(1.f), 5.1f);
    EXPECT_FLOAT_EQ(range.from_fraction(0.5f), 2.6f);
    EXPECT_FLOAT_EQ(range.to_fraction(2.6f), 0.5f);
}

TEST(SliderTest, SnapsToStep) {
    const SliderRange range{.min = 0.25f, .max = 5.f, .step = 0.25f};

    EXPECT_FLOAT_EQ(range.from_fraction(0.1f), 0.75f);  // 0.725 -> 0.75
    EXPECT_FLOAT_EQ(range.from_fraction(0.999f), 5.f);
}

TEST(SliderTest, ClampsOutOfRange) {
    const SliderRange range{.min = 1.f, .max = 2.f};

    EXPECT_FLOAT_EQ(range.from_fraction(-3.f), 1.f);
    EXPECT_FLOAT_EQ(range.from_fraction(7.f), 2.f);
    EXPECT_FLOAT_EQ(range.to_fraction(10.f), 1.f);
    EXPECT_FLOAT_EQ(range.to_fraction(-10.f), 0.f);
}

TEST(SliderTest, RoundTripsValuesOnTheGrid) {
    const SliderRange range{.min = 0.05f, .max = 1.f, .step = 0.01f};

    for (float v = 0.05f; v <= 1.f; v += 0.07f) {
        const float snapped = range.from_fraction(range.to_fraction(v));
        EXPECT_NEAR(snapped, v, 0.005f + 1e-5f);
    }
}

}
}
