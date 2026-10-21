#include <game/help_sketch.h>

#include <asset_ids.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace game {
namespace {

constexpr float kMinTextSize = 9.f;
// Average glyph advance of the UI font, as a fraction of the font size. Only used to centre text.
constexpr float kAdvance = 0.56f;

std::size_t glyph_count(std::string_view value) {
    std::size_t count = 0;
    for (const char c : value) {
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

}

HelpSketch::HelpSketch(engine::ui::IDrawList& list, const engine::render::Rect& content,
        const engine::loc::Catalog& catalog) :
    list_(list),
    catalog_(catalog),
    height_(content.h) {
    scale_ = std::min(1.f, content.w / kDesignWidth);
    origin_ = {(content.w - kDesignWidth * scale_) * 0.5f, 0.f};
    if (scale_ < 1.f) {
        height_ = content.h / scale_;
    }
}

glm::vec2 HelpSketch::map(glm::vec2 p) const {
    return origin_ + p * scale_;
}

glm::vec2 HelpSketch::direction(float radians) {
    return {std::cos(radians), -std::sin(radians)};
}

void HelpSketch::line(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke) {
    list_.line(map(from), map(to), color, std::max(1.f, scaled(stroke)));
}

void HelpSketch::dashed(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke, float dash) {
    const glm::vec2 delta = to - from;
    const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length < 1e-3f) {
        return;
    }
    const glm::vec2 unit = delta / length;
    for (float at = 0.f; at < length; at += dash * 1.8f) {
        line(from + unit * at, from + unit * std::min(at + dash, length), color, stroke);
    }
}

void HelpSketch::polyline(std::initializer_list<glm::vec2> points, glm::vec4 color, float stroke) {
    const glm::vec2* previous = nullptr;
    for (const glm::vec2& point : points) {
        if (previous != nullptr) {
            line(*previous, point, color, stroke);
        }
        previous = &point;
    }
}

void HelpSketch::arrow(glm::vec2 from, glm::vec2 to, glm::vec4 color, float stroke, float head) {
    const glm::vec2 delta = to - from;
    const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length < 1e-3f) {
        return;
    }
    const glm::vec2 unit = delta / length;
    const float size = std::min(head, length * 0.6f);
    const glm::vec2 side{-unit.y, unit.x};
    const glm::vec2 base = to - unit * size;
    line(from, base + unit * (size * 0.4f), color, stroke);
    // A filled head: a fan of lines from the base to the tip.
    constexpr int kFan = 5;
    for (int i = -kFan; i <= kFan; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kFan);
        line(base + side * (size * 0.42f * t), to, color, stroke * 0.8f);
    }
}

void HelpSketch::rect(glm::vec2 top_left, glm::vec2 size, glm::vec4 color, float stroke, float radius) {
    const glm::vec2 a = map(top_left);
    list_.stroke_rect({a.x, a.y, scaled(size.x), scaled(size.y)}, color, std::max(1.f, scaled(stroke)),
            scaled(radius));
}

void HelpSketch::fill(glm::vec2 top_left, glm::vec2 size, glm::vec4 color, float radius) {
    const glm::vec2 a = map(top_left);
    list_.fill_rect({a.x, a.y, scaled(size.x), scaled(size.y)}, color, scaled(radius));
}

void HelpSketch::circle(glm::vec2 center, float radius, glm::vec4 color, float stroke) {
    constexpr float kTwoPi = 2.f * std::numbers::pi_v<float>;
    list_.arc(map(center), scaled(radius), 0.f, kTwoPi, color, std::max(1.f, scaled(stroke)));
}

void HelpSketch::disc(glm::vec2 center, float radius, glm::vec4 color) {
    fill(center - glm::vec2(radius), glm::vec2(radius * 2.f), color, radius);
}

void HelpSketch::arc(glm::vec2 center, float radius, float from_angle, float to_angle, glm::vec4 color, float stroke) {
    constexpr float kQuarter = std::numbers::pi_v<float> * 0.5f;
    list_.arc(map(center), scaled(radius), kQuarter - to_angle, kQuarter - from_angle, color,
            std::max(1.f, scaled(stroke)));
}

void HelpSketch::charge(glm::vec2 center, bool positive, float radius) {
    const glm::vec4 color = positive ? sketch::kPlus : sketch::kMinus;
    disc(center, radius, color);
    const glm::vec4 ink{0.05f, 0.06f, 0.10f, 1.f};
    const float arm = radius * 0.5f;
    line(center - glm::vec2(arm, 0.f), center + glm::vec2(arm, 0.f), ink, radius * 0.22f);
    if (positive) {
        line(center - glm::vec2(0.f, arm), center + glm::vec2(0.f, arm), ink, radius * 0.22f);
    }
}

void HelpSketch::out_of_page(glm::vec2 center, glm::vec4 color, float radius) {
    circle(center, radius, color, 2.f);
    disc(center, radius * 0.3f, color);
}

void HelpSketch::into_page(glm::vec2 center, glm::vec4 color, float radius) {
    circle(center, radius, color, 2.f);
    const float arm = radius * 0.62f;
    line(center + glm::vec2(-arm, -arm), center + glm::vec2(arm, arm), color, 2.f);
    line(center + glm::vec2(-arm, arm), center + glm::vec2(arm, -arm), color, 2.f);
}

void HelpSketch::text(std::string_view value, glm::vec2 top_left, glm::vec4 color, float size) {
    list_.set_font(assets::fonts::Inter_Regular, std::max(kMinTextSize, scaled(size)));
    list_.text(value, map(top_left), color);
}

void HelpSketch::text_centered(std::string_view value, glm::vec2 center, glm::vec4 color, float size) {
    const float width = static_cast<float>(glyph_count(value)) * size * kAdvance;
    text(value, center - glm::vec2(width * 0.5f, size * 0.55f), color, size);
}

void HelpSketch::label(std::string_view key, glm::vec2 top_left, glm::vec4 color, float size) {
    text(catalog_.text(key).text, top_left, color, size);
}

void HelpSketch::label_centered(std::string_view key, glm::vec2 center, glm::vec4 color, float size) {
    text_centered(catalog_.text(key).text, center, color, size);
}

}
