#pragma once

#include <algorithm>

namespace game {

inline constexpr float kMinTimeScale = 0.125f;
inline constexpr float kMaxTimeScale = 8.f;

struct SimClock {
    bool paused = false;
    bool step_requested = false;
    float scale = 1.f;

    void set_scale(float value) {
        scale = std::clamp(value, kMinTimeScale, kMaxTimeScale);
    }
};

}
