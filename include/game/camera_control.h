#pragma once

#include <glm/vec3.hpp>

namespace game {

inline constexpr float kMinOrthoHalf = 2.f;
inline constexpr float kMaxOrthoHalf = 60.f;
inline constexpr float kZoomStep = 1.15f;
inline constexpr float kDefaultOrthoHalf = 10.f;

struct CameraView {
    glm::vec3 position{0.f};
    float ortho_half = 10.f;
};

// Keeps `anchor` fixed on screen. Ortho size stays in [kMinOrthoHalf, kMaxOrthoHalf].
[[nodiscard]] CameraView zoom_about(const CameraView& view, glm::vec3 anchor, float notches);

}
