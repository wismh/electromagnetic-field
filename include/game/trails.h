#pragma once

#include <game/charge.h>

#include <glm/vec3.hpp>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <vector>

namespace game {

struct TrailPoint {
    glm::vec3 position{0.f};
    float time = 0.f;  // simulation time when recorded
};

struct Trail {
    std::uint32_t id = 0;
    float q = 0.f;
    std::deque<TrailPoint> points;  // oldest first
};

// Recent path of every moving charge, keyed by Charge::id so trails survive index shifts when other
// charges are removed. Driven by simulation time, so trails freeze while paused.
class Trails {
public:
    // Points older than this many simulation seconds are dropped.
    float duration = 6.f;
    // A new point is recorded once the charge has moved this far from the last one.
    float min_spacing = 0.05f;
    // A move longer than this between records is a teleport (drag, edit), not motion: restart the trail.
    float jump_distance = 2.f;
    std::size_t max_points = 800;

    void record(std::span<const Charge> charges, float time);
    void clear();

    [[nodiscard]] const std::vector<Trail>& trails() const {
        return trails_;
    }

private:
    std::vector<Trail> trails_;
};

}
