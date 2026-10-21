#pragma once

#include <engine/loc/catalog.h>
#include <engine/render/commands.h>
#include <engine/ui/draw_list.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <initializer_list>
#include <string>
#include <string_view>

namespace game {

// Colours of the schematic figures in the reference. They follow help.css and the charge colours in the scene.
namespace sketch {
inline constexpr glm::vec4 kText{0.84f, 0.86f, 0.90f, 1.f};
inline constexpr glm::vec4 kWhite{1.f, 1.f, 1.f, 1.f};
inline constexpr glm::vec4 kMuted{0.55f, 0.58f, 0.66f, 1.f};
inline constexpr glm::vec4 kFaint{1.f, 1.f, 1.f, 0.16f};
inline constexpr glm::vec4 kPlus{1.f, 0.55f, 0.30f, 1.f};
inline constexpr glm::vec4 kMinus{0.36f, 0.62f, 1.f, 1.f};
inline constexpr glm::vec4 kField{0.51f, 0.55f, 0.97f, 1.f};
inline constexpr glm::vec4 kForce{0.98f, 0.80f, 0.30f, 1.f};
inline constexpr glm::vec4 kGood{0.20f, 0.83f, 0.60f, 1.f};
inline constexpr glm::vec4 kBad{0.97f, 0.44f, 0.44f, 1.f};
}

// Draws a figure laid out on a fixed design canvas (kDesignWidth x height). The canvas is scaled down to fit a narrow
// reference window and centred. Text is positioned by its top-left corner unless a centred call is used.
class HelpSketch {
public:
    static constexpr float kDesignWidth = 640.f;

    HelpSketch(engine::ui::IDrawList& list, const engine::render::Rect& content, const engine::loc::Catalog& catalog);

    [[nodiscard]] static constexpr float width() {
        return kDesignWidth;
    }
    [[nodiscard]] float height() const {
        return height_;
    }

    void line(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke = 2.f);
    void dashed(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke = 1.5f, float dash = 6.f);
    void polyline(std::initializer_list<glm::vec2> points, glm::vec4 color, float stroke = 2.f);
    void arrow(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke = 2.5f, float head = 10.f);
    void rect(glm::vec2 top_left, glm::vec2 size, glm::vec4 color, float stroke = 2.f, float radius = 0.f);
    void fill(glm::vec2 top_left, glm::vec2 size, glm::vec4 color, float radius = 0.f);
    void circle(glm::vec2 center, float radius, glm::vec4 color, float stroke = 2.f);
    void disc(glm::vec2 center, float radius, glm::vec4 color);
    void arc(glm::vec2 center, float radius, float from_angle, float to_angle, glm::vec4 color, float stroke = 2.f);
    // A sign disc: orange "+" or blue "-".
    void charge(glm::vec2 center, bool positive, float radius = 12.f);
    // Out of the page (a dot) or into it (a cross) in a ring, the usual way to draw a vector normal to the figure.
    void out_of_page(glm::vec2 center, glm::vec4 color, float radius = 9.f);
    void into_page(glm::vec2 center, glm::vec4 color, float radius = 9.f);
    void text(std::string_view value, glm::vec2 top_left, glm::vec4 color = sketch::kText, float size = 15.f);
    void text_centered(std::string_view value, glm::vec2 center, glm::vec4 color = sketch::kText, float size = 15.f);
    // The same, for a word of the active language: `key` is a string-table id (`help.fig.*`).
    void label(std::string_view key, glm::vec2 top_left, glm::vec4 color = sketch::kMuted, float size = 14.f);
    void label_centered(std::string_view key, glm::vec2 center, glm::vec4 color = sketch::kMuted, float size = 14.f);

    // Unit vector for an angle measured from +x, counter-clockwise on the screen (y points up in the figure).
    [[nodiscard]] static glm::vec2 direction(float radians);

private:
    [[nodiscard]] glm::vec2 map(glm::vec2 p) const;
    [[nodiscard]] float scaled(float value) const {
        return value * scale_;
    }

    engine::ui::IDrawList& list_;
    const engine::loc::Catalog& catalog_;
    glm::vec2 origin_{};
    float scale_ = 1.f;
    float height_ = 0.f;
};

}
