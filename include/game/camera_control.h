#pragma once

#include <glm/vec3.hpp>

namespace game {

inline constexpr float kMinOrthoHalf = 2.f;
inline constexpr float kMaxOrthoHalf = 60.f;
inline constexpr float kZoomStep = 1.15f;

struct CameraView {
    glm::vec3 position{0.f};
    float ortho_half = 10.f;  // half-height of the visible area in world units
};

// Zooms by kZoomStep^-notches (wheel up = zoom in) while keeping the world point `anchor`
// at the same place on screen. The ortho size is clamped to [kMinOrthoHalf, kMaxOrthoHalf].
[[nodiscard]] CameraView zoom_about(const CameraView& view, glm::vec3 anchor, float notches);

}
