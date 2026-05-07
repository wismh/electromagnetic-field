#include <game/camera_control.h>

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

namespace game {
namespace {

// Screen-space offset of `point` relative to the view centre, in units of the half-height.
glm::vec3 screen_offset(const CameraView& view, glm::vec3 point) {
    return (point - view.position) / view.ortho_half;
}

TEST(CameraControlTest, WheelUpZoomsIn) {
    const CameraView view{.position = {0.f, 0.f, 0.f}, .ortho_half = 10.f};

    EXPECT_LT(zoom_about(view, {0.f, 0.f, 0.f}, 1.f).ortho_half, 10.f);
    EXPECT_GT(zoom_about(view, {0.f, 0.f, 0.f}, -1.f).ortho_half, 10.f);
}

TEST(CameraControlTest, AnchorStaysUnderCursor) {
    const CameraView view{.position = {1.f, -2.f, 0.f}, .ortho_half = 10.f};
    const glm::vec3 anchor{6.f, 3.f, 0.f};

    const CameraView zoomed = zoom_about(view, anchor, 3.f);

    EXPECT_NEAR(glm::length(screen_offset(zoomed, anchor) - screen_offset(view, anchor)), 0.f, 1e-5f);
}

TEST(CameraControlTest, ZoomIsClampedAndAnchorStillHolds) {
    const CameraView view{.position = {0.f, 0.f, 0.f}, .ortho_half = 10.f};
    const glm::vec3 anchor{4.f, -1.f, 0.f};

    const CameraView in = zoom_about(view, anchor, 100.f);
    const CameraView out = zoom_about(view, anchor, -100.f);

    EXPECT_FLOAT_EQ(in.ortho_half, kMinOrthoHalf);
    EXPECT_FLOAT_EQ(out.ortho_half, kMaxOrthoHalf);
    EXPECT_NEAR(glm::length(screen_offset(in, anchor) - screen_offset(view, anchor)), 0.f, 1e-5f);
}

TEST(CameraControlTest, CameraDepthIsPreserved) {
    const CameraView view{.position = {0.f, 0.f, 5.f}, .ortho_half = 10.f};

    EXPECT_EQ(zoom_about(view, {1.f, 1.f, 0.f}, 2.f).position.z, 5.f);
}

}
}
