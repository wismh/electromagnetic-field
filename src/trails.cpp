#include <game/trails.h>

#include <glm/geometric.hpp>

#include <algorithm>

namespace game {

void Trails::record(std::span<const Charge> charges, float time) {
    for (const Charge& c : charges) {
        if (c.id == 0) {
            continue;
        }
        auto it = std::find_if(trails_.begin(), trails_.end(), [&](const Trail& t) { return t.id == c.id; });
        if (it == trails_.end()) {
            if (c.fixed) {
                continue;
            }
            trails_.push_back(Trail{.id = c.id});
            it = std::prev(trails_.end());
        }
        Trail& trail = *it;
        trail.q = c.q;
        if (c.fixed) {
            continue;
        }
        if (!trail.points.empty()) {
            const float moved = glm::length(c.position - trail.points.back().position);
            if (moved > jump_distance) {
                trail.points.clear();
            } else if (moved < min_spacing) {
                continue;
            }
        }
        trail.points.push_back(TrailPoint{.position = c.position, .time = time});
    }

    for (Trail& trail : trails_) {
        while (!trail.points.empty() &&
                (trail.points.front().time < time - duration || trail.points.size() > max_points)) {
            trail.points.pop_front();
        }
    }
    std::erase_if(trails_, [&](const Trail& t) {
        const bool alive = std::any_of(charges.begin(), charges.end(), [&](const Charge& c) { return c.id == t.id; });
        return !alive || t.points.empty();
    });
}

void Trails::clear() {
    trails_.clear();
}

}
