#include <game/camera_control.h>

#include <algorithm>
#include <cmath>

namespace game {

CameraView zoom_about(const CameraView& view, glm::vec3 anchor, float notches) {
    const float target = view.ortho_half * std::pow(kZoomStep, -notches);
    const float ortho_half = std::clamp(target, kMinOrthoHalf, kMaxOrthoHalf);
    const float ratio = ortho_half / view.ortho_half;
    // A point at offset d from the camera is drawn at d / ortho on screen; keeping the anchor's
    // screen offset constant means its offset from the camera scales with the ortho size.
    glm::vec3 position = anchor - (anchor - view.position) * ratio;
    position.z = view.position.z;
    return CameraView{.position = position, .ortho_half = ortho_half};
}

}
