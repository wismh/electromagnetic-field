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

constexpr glm::vec4 kFieldFaint{0.51f, 0.55f, 0.97f, 0.35f};

// A faint grid of crosses: a uniform field going into the page.
void field_into_page(HelpSketch& s, glm::vec2 top_left, int columns, int rows, float step) {
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const glm::vec2 at = top_left + glm::vec2(step * static_cast<float>(column), step * static_cast<float>(row));
            s.line(at + glm::vec2(-4.f, -4.f), at + glm::vec2(4.f, 4.f), kFieldFaint, 1.6f);
            s.line(at + glm::vec2(-4.f, 4.f), at + glm::vec2(4.f, -4.f), kFieldFaint, 1.6f);
        }
    }
}

// A short arrow along a circle, pointing counter-clockwise or clockwise.
void turn_arrow(HelpSketch& s, glm::vec2 center, float radius, float angle, bool counter_clockwise, glm::vec4 color) {
    const glm::vec2 tangent = HelpSketch::direction(angle + (counter_clockwise ? kPi : -kPi) * 0.5f);
    const glm::vec2 p = center + HelpSketch::direction(angle) * radius;
    s.arrow(p - tangent * 9.f, p + tangent * 9.f, color, 2.5f, 11.f);
}

}

void draw_wire_field(HelpSketch& s) {
    // Left: a wire with the current coming out of the page. B circles it counter-clockwise.
    const glm::vec2 c{170.f, 140.f};
    const float radii[] = {34.f, 64.f, 94.f};
    for (const float r : radii) {
        s.circle(c, r, kFieldFaint, 1.8f);
        turn_arrow(s, c, r, rad(90.f), true, kField);
        turn_arrow(s, c, r, rad(270.f), true, kField);
    }
    s.out_of_page(c, kForce, 11.f);
    s.text("I", c + glm::vec2(18.f, 8.f), kForce, 17.f);
    s.text("B", c + glm::vec2(100.f, -26.f), kField, 17.f);
    s.label("help.fig.field_circles", {70.f, 250.f}, kMuted, 14.f);

    // Right: the same seen from the side. The thumb along I, the fingers along B.
    const float x = 470.f;
    s.arrow({x, 230.f}, {x, 28.f}, kForce, 4.f, 14.f);
    s.text("I", {x + 12.f, 34.f}, kForce, 17.f);
    for (const float y : {96.f, 168.f}) {
        constexpr int kSteps = 36;
        glm::vec2 previous = glm::vec2(x + 70.f, y);
        for (int i = 1; i <= kSteps; ++i) {
            const float angle = 2.f * kPi * static_cast<float>(i) / static_cast<float>(kSteps);
            const glm::vec2 next{x + 70.f * std::cos(angle), y - 20.f * std::sin(angle)};
            // The far half is dashed lighter; the near half is solid.
            const bool far_half = std::sin(angle) > 0.f;
            s.line(previous, next, far_half ? kFieldFaint : kField, 2.2f);
            previous = next;
        }
        // The near side runs to the right, the far side to the left.
        s.arrow({x - 14.f, y + 20.f}, {x + 22.f, y + 20.f}, kField, 2.5f, 10.f);
    }
    s.text("B", {x + 80.f, 112.f}, kField, 17.f);
}

void draw_lorentz_directions(HelpSketch& s) {
    // B goes into the page in both panels. v goes right; the force turns left for q > 0 and right for q < 0.
    const struct {
        float x;
        bool positive;
    } panels[] = {{170.f, true}, {470.f, false}};
    for (const auto& panel : panels) {
        field_into_page(s, {panel.x - 130.f, 40.f}, 11, 6, 26.f);
        const glm::vec2 q{panel.x - 50.f, 140.f};
        s.charge(q, panel.positive, 14.f);
        s.arrow(q + glm::vec2(18.f, 0.f), q + glm::vec2(120.f, 0.f), kGood, 3.f, 12.f);
        s.text("v", q + glm::vec2(84.f, 10.f), kGood, 17.f);
        const float direction = panel.positive ? -1.f : 1.f;
        s.arrow(q + glm::vec2(0.f, direction * 22.f), q + glm::vec2(0.f, direction * 96.f), kForce, 3.f, 12.f);
        s.text("F", q + glm::vec2(12.f, direction * 62.f - 10.f), kForce, 17.f);
        s.text(panel.positive ? "q > 0" : "q < 0", {panel.x - 30.f, 232.f}, kText, 17.f);
    }
    s.into_page({320.f, 24.f}, kField, 9.f);
    s.text("B", {336.f, 14.f}, kField, 17.f);
}

void draw_cyclotron_orbit(HelpSketch& s) {
    field_into_page(s, {30.f, 28.f}, 15, 8, 31.f);
    const glm::vec2 c{250.f, 130.f};
    constexpr float kRadius = 88.f;
    s.circle(c, kRadius, kText, 2.5f);
    const glm::vec2 q = c + glm::vec2(0.f, kRadius);
    s.charge(q, true, 12.f);
    s.arrow(q + glm::vec2(16.f, 0.f), q + glm::vec2(96.f, 0.f), kGood, 3.f, 12.f);
    s.text("v", q + glm::vec2(70.f, 6.f), kGood, 17.f);
    s.arrow(q + glm::vec2(0.f, -16.f), q + glm::vec2(0.f, -70.f), kForce, 3.f, 12.f);
    s.text("F", q + glm::vec2(10.f, -50.f), kForce, 17.f);
    s.dashed(c, c + HelpSketch::direction(rad(150.f)) * kRadius, kText, 1.6f, 6.f);
    s.text("r", c + HelpSketch::direction(rad(150.f)) * 44.f + glm::vec2(-6.f, -20.f), kText, 17.f);
    s.disc(c, 3.f, kText);

    const float x = 400.f;
    s.text("F = q v B", {x, 40.f}, kForce, 19.f);
    s.text("m v² / r = q v B", {x, 82.f}, kText, 19.f);
    s.text("r = m v / (q B)", {x, 124.f}, kWhite, 19.f);
    s.text("T = 2π m / (q B)", {x, 166.f}, kGood, 19.f);
}

void draw_exb_cycloid(HelpSketch& s) {
    field_into_page(s, {30.f, 30.f}, 15, 8, 40.f);
    const float top = 52.f;
    const float radius = 36.f;
    glm::vec2 previous{60.f, top};
    for (int i = 1; i <= 90; ++i) {
        const float t = 4.f * kPi * static_cast<float>(i) / 90.f;
        const glm::vec2 next{60.f + radius * (t - std::sin(t)), top + radius * (1.f - std::cos(t))};
        s.line(previous, next, kGood, 3.f);
        previous = next;
    }
    s.disc({60.f, top}, 6.f, kPlus);
    // E points down, the drift goes to the right.
    s.arrow({22.f, 62.f}, {22.f, 170.f}, kForce, 3.f, 12.f);
    s.text("E", {32.f, 160.f}, kForce, 18.f);
    s.dashed({60.f, top - 12.f}, {60.f + radius * 4.f * kPi + 10.f, top - 12.f}, kMuted, 1.5f, 8.f);
    s.arrow({330.f, 30.f}, {470.f, 30.f}, kWhite, 3.f, 12.f);
    s.text("v = E / B", {350.f, 6.f}, kWhite, 17.f);
    s.into_page({560.f, 216.f}, kField, 9.f);
    s.text("B", {578.f, 204.f}, kField, 17.f);
}

void draw_coil_field(HelpSketch& s) {
    // A current loop seen from the side: the current comes out of the page on the left and goes into it on the right.
    const float y = 140.f;
    const glm::vec2 left{240.f, y};
    const glm::vec2 right{400.f, y};
    s.line(left + glm::vec2(0.f, 0.f), right, kForce, 2.f);
    s.out_of_page(left, kForce, 11.f);
    s.into_page(right, kForce, 11.f);
    s.text("I", left + glm::vec2(-26.f, 4.f), kForce, 17.f);
    // The field goes up through the loop and comes back around.
    s.arrow({320.f, y + 100.f}, {320.f, y - 100.f}, kField, 3.f, 12.f);
    for (const float a : {130.f, 190.f}) {
        const float h = a * 0.45f + 40.f;
        for (const float sign : {1.f, -1.f}) {
            glm::vec2 previous{320.f, y - h};
            for (int i = 1; i <= 36; ++i) {
                const float t = kPi * static_cast<float>(i) / 36.f;
                const glm::vec2 next{320.f + sign * a * std::sin(t), y - h * std::cos(t)};
                s.line(previous, next, kField, 2.f);
                previous = next;
            }
            s.arrow({320.f + sign * a - 0.f, y - 6.f}, {320.f + sign * a, y + 14.f}, kField, 2.2f, 10.f);
        }
    }
    s.text("B", {332.f, y - 92.f}, kField, 18.f);
}

}
