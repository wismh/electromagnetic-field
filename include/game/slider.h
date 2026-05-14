#pragma once

#include <algorithm>
#include <cmath>

namespace game {

// Linear slider range with snapping. The UI drag binding works in [0, 1] fractions.
struct SliderRange {
    float min = 0.f;
    float max = 1.f;
    float step = 0.f;  // 0 = continuous

    [[nodiscard]] float clamp(float value) const {
        return std::clamp(value, min, max);
    }

    [[nodiscard]] float to_fraction(float value) const {
        return max > min ? (clamp(value) - min) / (max - min) : 0.f;
    }

    [[nodiscard]] float from_fraction(float fraction) const {
        float value = min + std::clamp(fraction, 0.f, 1.f) * (max - min);
        if (step > 0.f) {
            value = min + std::round((value - min) / step) * step;
        }
        return clamp(value);
    }
};

}
