#pragma once

#include <engine/ui/canvas.h>

#include <game/overlay_pop.h>

#include <glm/vec2.hpp>

#include <algorithm>

namespace game {

struct PlacedWindow {
    int left = 0;
    int width = 0;
};

// Explicit width: a max-width parent measures children against its unclamped width.
[[nodiscard]] inline PlacedWindow place_window(glm::ivec2 window_size, int margin, int max_width) {
    const int width = std::max(0, std::min(window_size.x - 2 * margin, max_width));
    return PlacedWindow{.left = (window_size.x - width) / 2, .width = width};
}

// A closed full-window canvas would take every click. While closed it is an empty rect.
inline void apply_overlay_canvas(engine::ui::UiCanvas& canvas, const OverlayPop& pop) {
    canvas.fit = pop.is_open() ? engine::ui::UiFit::FillWindow : engine::ui::UiFit::Fixed;
    if (!pop.is_open()) {
        canvas.rect = {};
    }
}

}
