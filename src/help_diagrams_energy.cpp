#include <game/help_diagrams.h>

#include <cmath>
#include <numbers>

namespace game {
namespace {

using namespace sketch;

constexpr float kPi = std::numbers::pi_v<float>;

constexpr float rad(float degrees) {
    return degrees * kPi / 180.f;
}

// Plots y(x) as a polyline of short segments.
template<typename F>
void plot(HelpSketch& s, float from, float to, F&& point, glm::vec4 color, float stroke = 3.f) {
    constexpr int kSteps = 48;
    glm::vec2 previous = point(from);
    for (int i = 1; i <= kSteps; ++i) {
        const float t = from + (to - from) * static_cast<float>(i) / static_cast<float>(kSteps);
        const glm::vec2 next = point(t);
        s.line(previous, next, color, stroke);
        previous = next;
    }
}

void axes(HelpSketch& s, glm::vec2 origin, float width, float up, float down, const char* x_name, const char* y_name) {
    s.arrow(origin + glm::vec2(0.f, down), origin + glm::vec2(0.f, -up), kMuted, 1.8f, 8.f);
    s.arrow(origin, origin + glm::vec2(width, 0.f), kMuted, 1.8f, 8.f);
    s.text(x_name, origin + glm::vec2(width - 6.f, 8.f), kText, 16.f);
    s.text(y_name, origin + glm::vec2(10.f, -up - 4.f), kText, 16.f);
}

}

void draw_equipotentials(HelpSketch& s) {
    const glm::vec2 c{170.f, 130.f};
    // phi = k Q / r: equal steps of phi sit closer and closer to the charge.
    for (const float radius : {30.f, 40.f, 60.f, 120.f}) {
        s.circle(c, radius, kText, 1.8f);
    }
    s.charge(c, true, 13.f);
    for (int i = 0; i < 4; ++i) {
        const glm::vec2 d = HelpSketch::direction(rad(45.f + 90.f * static_cast<float>(i)));
        s.arrow(c + d * 62.f, c + d * 150.f, kField, 2.5f, 10.f);
    }
    s.text("E", c + HelpSketch::direction(rad(45.f)) * 160.f + glm::vec2(2.f, -18.f), kField, 17.f);
    s.text("φ = 4, 3, 2, 1", {360.f, 70.f}, kWhite, 18.f);
    s.label("help.fig.phi_outward", {360.f, 96.f}, kMuted, 14.f);
    s.label("help.fig.e_perpendicular", {360.f, 128.f}, kField, 14.f);
    s.label("help.fig.crowded", {360.f, 156.f}, kText, 14.f);
    s.label("help.fig.strong_field", {360.f, 178.f}, kText, 14.f);
}

void draw_capacitor(HelpSketch& s) {
    const float left = 130.f;
    const float right = 470.f;
    const float top = 52.f;
    const float bottom = 208.f;
    s.fill({left, top - 5.f}, {right - left, 10.f}, kPlus, 3.f);
    s.fill({left, bottom - 5.f}, {right - left, 10.f}, kMinus, 3.f);
    for (int i = 0; i < 9; ++i) {
        const float x = left + 22.f + 37.f * static_cast<float>(i);
        s.text_centered("+", {x, top - 18.f}, kPlus, 15.f);
        s.text_centered("−", {x, bottom + 20.f}, kMinus, 15.f);
    }
    for (int i = 0; i < 6; ++i) {
        const float x = left + 36.f + 58.f * static_cast<float>(i);
        s.arrow({x, top + 14.f}, {x, bottom - 14.f}, kField, 2.5f, 10.f);
    }
    for (int i = 1; i < 4; ++i) {
        const float y = top + (bottom - top) * static_cast<float>(i) / 4.f;
        s.dashed({left, y}, {right, y}, kFaint, 1.5f, 6.f);
    }
    s.text("A", {left - 26.f, top - 12.f}, kText, 17.f);
    s.text("B", {left - 26.f, bottom - 12.f}, kText, 17.f);
    s.line({right + 24.f, top}, {right + 24.f, bottom}, kText, 1.5f);
    s.line({right + 18.f, top}, {right + 30.f, top}, kText, 1.5f);
    s.line({right + 18.f, bottom}, {right + 30.f, bottom}, kText, 1.5f);
    s.text("d", {right + 36.f, (top + bottom) * 0.5f - 10.f}, kText, 17.f);
    s.text("Δφ = φA − φB = E · d", {110.f, 246.f}, kWhite, 18.f);
}

void draw_potential_slope(HelpSketch& s) {
    const glm::vec2 o{60.f, 215.f};
    axes(s, o, 540.f, 190.f, 4.f, "r", "φ");
    const auto point = [&](float r) { return glm::vec2(o.x + 100.f * r, o.y - 90.f / r); };
    plot(s, 0.5f, 5.0f, point, kPlus, 3.f);

    // The slope at two points: steep near the charge, gentle far away. E is minus the slope.
    const float radii[] = {0.9f, 2.6f};
    for (const float r : radii) {
        const glm::vec2 p = point(r);
        const float slope = (90.f / (r * r)) / 100.f;
        const glm::vec2 along = glm::vec2(1.f, slope);
        s.line(p - along * 55.f, p + along * 55.f, kGood, 2.f);
        s.disc(p, 5.f, kWhite);
        s.dashed(p, {p.x, o.y}, kFaint, 1.2f, 5.f);
        const float length = 46.f * slope + 6.f;
        s.arrow({p.x, o.y + 22.f}, {p.x + length, o.y + 22.f}, kField, 3.f, 9.f);
        s.text("E", {p.x - 22.f, o.y + 12.f}, kField, 16.f);
    }
    s.text("E = −dφ/dr", {330.f, 40.f}, kWhite, 19.f);
    s.label("help.fig.slope_is_field", {330.f, 68.f}, kMuted, 15.f);
}

void draw_pair_energy_curves(HelpSketch& s) {
    const glm::vec2 o{50.f, 135.f};
    axes(s, o, 560.f, 112.f, 112.f, "r", "U");
    const auto like = [&](float r) { return glm::vec2(o.x + 100.f * r, o.y - 75.f / r); };
    const auto unlike = [&](float r) { return glm::vec2(o.x + 100.f * r, o.y + 75.f / r); };
    plot(s, 0.75f, 5.2f, like, kPlus, 3.f);
    plot(s, 0.75f, 5.2f, unlike, kMinus, 3.f);
    s.text("q1 q2 > 0", {o.x + 118.f, 22.f}, kPlus, 15.f);
    s.text("q1 q2 < 0", {o.x + 118.f, 232.f}, kMinus, 15.f);

    // Two like charges fly at one another: all the kinetic energy turns into U at the closest approach.
    const float energy = 36.f;
    s.dashed({o.x, o.y - energy}, {o.x + 540.f, o.y - energy}, kForce, 1.8f, 7.f);
    s.text("K + U", {o.x + 470.f, o.y - energy - 24.f}, kForce, 15.f);
    const float r_min = 75.f / energy;
    s.dashed({o.x + 100.f * r_min, o.y - energy}, {o.x + 100.f * r_min, o.y}, kGood, 1.8f, 5.f);
    s.disc({o.x + 100.f * r_min, o.y - energy}, 5.f, kGood);
    s.text("r min", {o.x + 100.f * r_min - 20.f, o.y + 8.f}, kGood, 15.f);
    s.text("U = k q1 q2 / r", {o.x + 330.f, o.y - 74.f}, kWhite, 17.f);
}

void draw_energy_zero_levels(HelpSketch& s) {
    const glm::vec2 o{120.f, 70.f};
    // U(r) = -k / r, drawn from just above the zero line down toward the charge.
    const auto curve = [&](float r) { return glm::vec2(o.x + 90.f * r, o.y + 110.f / r); };
    plot(s, 0.7f, 5.5f, curve, kMinus, 3.f);
    s.line({o.x - 20.f, o.y}, {o.x + 470.f, o.y}, kMuted, 1.8f);
    s.text("r", {o.x + 474.f, o.y + 8.f}, kText, 16.f);

    // Two scales for the same energy: zero at infinity, or zero chosen elsewhere.
    const float ticks[] = {0.f, 1.f, 2.f};
    for (const float t : ticks) {
        const float y = o.y + 54.f * t;
        s.line({o.x - 26.f, y}, {o.x - 16.f, y}, kText, 1.6f);
        s.text_centered(t == 0.f ? "0" : (t == 1.f ? "−1" : "−2"), {o.x - 42.f, y}, kText, 15.f);
        s.line({o.x + 520.f, y}, {o.x + 530.f, y}, kField, 1.6f);
        s.text_centered(t == 0.f ? "3" : (t == 1.f ? "2" : "1"), {o.x + 550.f, y}, kField, 15.f);
    }
    s.line({o.x - 26.f, o.y}, {o.x - 26.f, o.y + 108.f}, kText, 1.6f);
    s.line({o.x + 530.f, o.y}, {o.x + 530.f, o.y + 108.f}, kField, 1.6f);
    s.label("help.fig.zero_infinity", {10.f, o.y + 126.f}, kText, 14.f);
    s.label("help.fig.zero_shifted", {400.f, o.y + 126.f}, kField, 14.f);
    // The same difference on both scales.
    for (const float t : {1.f, 2.f}) {
        const float y = o.y + 54.f * t;
        s.dashed({o.x - 16.f, y}, {o.x + 520.f, y}, kFaint, 1.4f, 6.f);
        s.disc(curve(110.f / (54.f * t)), 5.f, kWhite);
    }
    s.arrow({o.x + 330.f, o.y + 56.f}, {o.x + 330.f, o.y + 106.f}, kGood, 2.5f, 9.f);
    s.text("ΔU = 1", {o.x + 344.f, o.y + 70.f}, kGood, 18.f);
}

}
