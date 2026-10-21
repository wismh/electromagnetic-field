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

// Radial field arrows from a centre, between two radii.
void radial_arrows(HelpSketch& s, glm::vec2 center, float from, float to, int count, glm::vec4 color, float phase = 0.f) {
    for (int i = 0; i < count; ++i) {
        const glm::vec2 d = HelpSketch::direction(phase + 2.f * kPi * static_cast<float>(i) / static_cast<float>(count));
        s.arrow(center + d * from, center + d * to, color, 2.f, 8.f);
    }
}

}

void draw_flux_angle(HelpSketch& s) {
    // One patch with its normal and the field at an angle to it.
    const glm::vec2 patch{150.f, 190.f};
    s.line(patch - glm::vec2(90.f, 0.f), patch + glm::vec2(90.f, 0.f), kMuted, 5.f);
    s.dashed(patch, patch + glm::vec2(0.f, -150.f), kFaint, 1.5f);
    s.arrow(patch, patch + glm::vec2(0.f, -105.f), kGood, 3.f, 11.f);
    s.text("dA", patch + glm::vec2(8.f, -128.f), kGood, 16.f);
    const glm::vec2 field = HelpSketch::direction(rad(55.f));
    s.arrow(patch, patch + field * 130.f, kField, 3.f, 11.f);
    s.text("E", patch + field * 140.f + glm::vec2(2.f, -12.f), kField, 16.f);
    s.arc(patch, 52.f, rad(55.f), rad(90.f), kText, 1.5f);
    s.text("θ", patch + HelpSketch::direction(rad(72.f)) * 68.f - glm::vec2(5.f, 9.f), kText, 16.f);
    s.text("dΦ = E·dA·cos θ", {55.f, 22.f}, kText, 16.f);

    // Three special angles.
    struct Case {
        float x;
        const char* angle;
        const char* cosine;
        glm::vec4 color;
    };
    const Case cases[] = {
            {370.f, "θ = 0°", "+1", kGood},
            {490.f, "θ = 90°", "0", kMuted},
            {610.f, "θ = 180°", "−1", kBad},
    };
    for (int i = 0; i < 3; ++i) {
        const Case& c = cases[i];
        const glm::vec2 at{c.x - (i == 2 ? 0.f : 0.f), 150.f};
        s.line(at - glm::vec2(42.f, 0.f), at + glm::vec2(42.f, 0.f), kMuted, 4.f);
        s.dashed(at, at + glm::vec2(0.f, -62.f), kFaint, 1.2f, 4.f);
        if (i == 0) {
            s.arrow(at, at + glm::vec2(0.f, -62.f), kField, 3.f, 9.f);
        } else if (i == 1) {
            s.arrow(at + glm::vec2(-40.f, -12.f), at + glm::vec2(40.f, -12.f), kField, 3.f, 9.f);
        } else {
            s.arrow(at + glm::vec2(0.f, -62.f), at, kField, 3.f, 9.f);
        }
        s.text_centered(c.angle, at + glm::vec2(0.f, 30.f), kText, 14.f);
        s.text_centered(c.cosine, at + glm::vec2(0.f, 54.f), c.color, 18.f);
    }
}

void draw_sphere_flux(HelpSketch& s) {
    const glm::vec2 c{250.f, 140.f};
    constexpr float kRadius = 66.f;
    s.circle(c, kRadius, kText, 2.f);
    radial_arrows(s, c, kRadius, kRadius + 40.f, 16, kField, rad(11.f));
    s.charge(c, true, 14.f);

    // The radius to a point of the sphere, and one patch there with its outward vector.
    const glm::vec2 on = c + HelpSketch::direction(rad(215.f)) * kRadius;
    s.dashed(c, on, kText, 1.5f);
    s.text("r", c + HelpSketch::direction(rad(215.f)) * 38.f + glm::vec2(-14.f, -2.f), kText, 16.f);

    const glm::vec2 patch = c + HelpSketch::direction(rad(35.f)) * kRadius;
    const glm::vec2 tangent = HelpSketch::direction(rad(35.f) + kPi * 0.5f);
    s.line(patch - tangent * 14.f, patch + tangent * 14.f, kGood, 6.f);
    s.arrow(patch, patch + HelpSketch::direction(rad(35.f)) * 62.f, kGood, 3.f, 11.f);
    s.text("dA", patch + HelpSketch::direction(rad(35.f)) * 72.f + glm::vec2(-6.f, -22.f), kGood, 16.f);
    s.text("E", c + HelpSketch::direction(rad(70.f)) * (kRadius + 54.f) + glm::vec2(-4.f, -14.f), kField, 16.f);

    s.text("S = 4π r²", {430.f, 82.f}, kText, 18.f);
    s.text("Φ = Σ E·dA", {430.f, 118.f}, kText, 18.f);
    s.text("= E · 4π r²", {430.f, 154.f}, kWhite, 18.f);
}

void draw_sphere_area(HelpSketch& s) {
    const glm::vec2 c{150.f, 125.f};
    constexpr float kNear = 42.f;
    constexpr float kFar = 84.f;
    s.circle(c, kNear, kText, 2.f);
    s.circle(c, kFar, kText, 2.f);
    // The same eight lines pass both spheres.
    radial_arrows(s, c, 18.f, kFar + 44.f, 8, kField, rad(22.5f));
    s.charge(c, true, 13.f);
    s.text("r", c + glm::vec2(-kNear - 20.f, -12.f), kText, 15.f);
    s.text("2r", c + glm::vec2(-kFar - 28.f, -12.f), kText, 15.f);

    // A comparison of the two spheres.
    const float x0 = 360.f;
    s.text("r", {x0 + 110.f, 38.f}, kMuted, 16.f);
    s.text("2r", {x0 + 190.f, 38.f}, kMuted, 16.f);
    const struct {
        const char* name;
        const char* near;
        const char* far;
        glm::vec4 color;
    } rows[] = {
            {"E", "1", "1/4", kField},
            {"S", "1", "4", kForce},
            {"E·S", "1", "1", kGood},
    };
    for (int i = 0; i < 3; ++i) {
        const float y = 78.f + 52.f * static_cast<float>(i);
        s.text(rows[i].name, {x0, y}, rows[i].color, 20.f);
        s.text(rows[i].near, {x0 + 110.f, y}, kText, 20.f);
        s.text(rows[i].far, {x0 + 190.f, y}, kText, 20.f);
    }
    s.line({x0 - 10.f, 68.f + 52.f * 2.f}, {x0 + 250.f, 68.f + 52.f * 2.f}, kFaint, 1.5f);
}

void draw_solid_angle(HelpSketch& s) {
    const glm::vec2 c{50.f, 118.f};
    constexpr float kHalf = 0.22f;
    const glm::vec2 up = HelpSketch::direction(kHalf);
    const glm::vec2 down = HelpSketch::direction(-kHalf);
    s.dashed(c, c + up * 500.f, kMuted, 1.5f, 8.f);
    s.dashed(c, c + down * 500.f, kMuted, 1.5f, 8.f);
    s.charge(c, true, 12.f);

    // Two spherical patches in the same cone: the farther one is larger by the ratio of distances squared.
    s.arc(c, 130.f, -kHalf, kHalf, kGood, 5.f);
    s.arc(c, 260.f, -kHalf, kHalf, kForce, 5.f);
    s.text("dA", c + glm::vec2(138.f, -34.f), kGood, 16.f);
    s.text("4 dA", c + glm::vec2(268.f, -66.f), kForce, 16.f);
    // An oblique surface, farther again and tilted: still the same cone.
    const glm::vec2 a = c + up * 400.f;
    const glm::vec2 b = c + down * 470.f;
    s.line(a, b, kField, 5.f);
    s.text("dA'", a + glm::vec2(10.f, -20.f), kField, 16.f);
    s.text("dΩ", c + glm::vec2(50.f, -8.f), kText, 16.f);
    s.arc(c, 40.f, -kHalf, kHalf, kText, 1.5f);
    s.text("dΦ = E·dA = k q dΩ", {44.f, 236.f}, kWhite, 17.f);
    s.label("help.fig.same_for_all", {260.f, 238.f}, kMuted, 15.f);
}

void draw_outside_charge(HelpSketch& s) {
    const glm::vec2 body{410.f, 120.f};
    constexpr float kRadius = 88.f;
    s.circle(body, kRadius, kText, 2.5f);

    const glm::vec2 source{70.f, 120.f};
    s.charge(source, true, 12.f);
    constexpr float kHalf = 0.20f;
    // A thin cone leaves the charge, enters the surface on the near side and leaves on the far side.
    s.dashed(source, source + HelpSketch::direction(kHalf) * 520.f, kMuted, 1.5f, 8.f);
    s.dashed(source, source + HelpSketch::direction(-kHalf) * 520.f, kMuted, 1.5f, 8.f);
    s.arrow(source + glm::vec2(30.f, 0.f), source + glm::vec2(200.f, 0.f), kField, 2.5f, 10.f);

    const float near_distance = (body.x - source.x) - std::sqrt(kRadius * kRadius - 0.f);
    s.arc(source, near_distance, -kHalf, kHalf, kBad, 6.f);
    const float far_distance = (body.x - source.x) + kRadius;
    s.arc(source, far_distance, -kHalf, kHalf, kGood, 6.f);
    s.text("−", {near_distance + source.x + 10.f, 74.f}, kBad, 22.f);
    s.text("+", {far_distance + source.x + 10.f, 54.f}, kGood, 22.f);
    s.text("Φ  =  (−)  +  (+)  =  0", {170.f, 232.f}, kText, 18.f);
}

void draw_flux_superposition(HelpSketch& s) {
    const glm::vec2 body{250.f, 120.f};
    constexpr float kRadius = 96.f;
    s.circle(body, kRadius, kText, 2.5f);
    s.charge(body + glm::vec2(-30.f, -22.f), true, 12.f);
    s.charge(body + glm::vec2(34.f, 26.f), false, 12.f);
    s.text("q1", body + glm::vec2(-52.f, -52.f), kText, 15.f);
    s.text("q2", body + glm::vec2(48.f, 28.f), kText, 15.f);

    const glm::vec2 outside{520.f, 70.f};
    s.charge(outside, true, 12.f);
    s.text("q3", outside + glm::vec2(18.f, -26.f), kText, 15.f);
    for (int i = 0; i < 3; ++i) {
        const glm::vec2 target = body + glm::vec2(kRadius - 18.f, -40.f + 40.f * static_cast<float>(i));
        s.dashed(outside, target, kFaint, 1.5f, 6.f);
    }
    s.text("Φ  =  (q1 + q2) / ε0", {96.f, 232.f}, kWhite, 18.f);
    s.label("help.fig.outside_no_count", {420.f, 150.f});
}

void draw_gauss_cylinder(HelpSketch& s) {
    const float axis = 190.f;
    s.fill({axis - 3.f, 20.f}, {6.f, 230.f}, kPlus, 3.f);
    for (int i = 0; i < 7; ++i) {
        s.text_centered("+", {axis + 15.f, 40.f + 32.f * static_cast<float>(i)}, kPlus, 14.f);
    }
    const float wall = 80.f;
    s.rect({axis - wall, 70.f}, {2.f * wall, 140.f}, kText, 2.f);
    // The caps, seen edge-on: the field runs along them.
    s.line({axis - wall, 70.f}, {axis + wall, 70.f}, kMuted, 5.f);
    s.line({axis - wall, 210.f}, {axis + wall, 210.f}, kMuted, 5.f);
    for (int i = 0; i < 3; ++i) {
        const float y = 100.f + 40.f * static_cast<float>(i);
        s.arrow({axis - wall, y}, {axis - wall - 46.f, y}, kGood, 2.5f, 9.f);
        s.arrow({axis + wall, y}, {axis + wall + 46.f, y}, kGood, 2.5f, 9.f);
    }
    s.dashed({axis, 140.f}, {axis + wall, 140.f}, kText, 1.5f);
    s.text("r", {axis + 36.f, 118.f}, kText, 15.f);
    s.line({axis + wall + 74.f, 70.f}, {axis + wall + 74.f, 210.f}, kText, 1.5f);
    s.text("L", {axis + wall + 82.f, 132.f}, kText, 16.f);
    s.label("help.fig.caps_no_flux", {16.f, 34.f});

    s.text("Q = λ L", {420.f, 70.f}, kText, 18.f);
    s.text("S = 2π r L", {420.f, 104.f}, kText, 18.f);
    s.text("E · 2π r L = λ L / ε0", {420.f, 138.f}, kWhite, 18.f);
    s.text("E = λ / (2π ε0 r)", {420.f, 172.f}, kGood, 18.f);
}

void draw_gauss_pillbox(HelpSketch& s) {
    const float plane = 130.f;
    s.fill({50.f, plane - 3.f}, {540.f, 6.f}, kPlus, 3.f);
    for (int i = 0; i < 11; ++i) {
        s.text_centered("+", {70.f + 50.f * static_cast<float>(i), plane + 15.f}, kPlus, 13.f);
    }
    for (const float x : {100.f, 190.f, 290.f, 350.f, 560.f, 610.f}) {
        s.arrow({x, plane - 14.f}, {x, plane - 70.f}, kField, 2.f, 8.f);
        s.arrow({x, plane + 34.f}, {x, plane + 92.f}, kField, 2.f, 8.f);
    }
    const float left = 255.f;
    const float right = 385.f;
    s.dashed({left, plane - 44.f}, {left, plane + 44.f}, kText, 1.5f);
    s.dashed({right, plane - 44.f}, {right, plane + 44.f}, kText, 1.5f);
    s.line({left, plane - 44.f}, {right, plane - 44.f}, kGood, 5.f);
    s.line({left, plane + 44.f}, {right, plane + 44.f}, kGood, 5.f);
    s.text("A", {right + 10.f, plane - 64.f}, kGood, 16.f);
    s.text("A", {right + 10.f, plane + 36.f}, kGood, 16.f);
    s.label("help.fig.sides_no_flux", {right + 14.f, plane + 54.f}, kMuted, 13.f);
    s.text("σ", {58.f, plane - 30.f}, kPlus, 18.f);
    s.text("Q = σ A     2 E A = σ A / ε0     E = σ / (2 ε0)", {70.f, 246.f}, kWhite, 17.f);
}

void draw_gauss_conductor(HelpSketch& s) {
    const glm::vec2 top_left{190.f, 56.f};
    const glm::vec2 size{260.f, 170.f};
    s.fill(top_left, size, glm::vec4(1.f, 1.f, 1.f, 0.07f), 26.f);
    s.rect(top_left, size, kText, 2.5f, 26.f);
    // Excess charge sits on the surface.
    for (int i = 0; i < 9; ++i) {
        const float x = top_left.x + 20.f + 27.5f * static_cast<float>(i);
        s.text_centered("+", {x, top_left.y + 12.f}, kPlus, 14.f);
        s.text_centered("+", {x, top_left.y + size.y - 12.f}, kPlus, 14.f);
    }
    for (int i = 0; i < 3; ++i) {
        const float y = top_left.y + 52.f + 33.f * static_cast<float>(i);
        s.text_centered("+", {top_left.x + 12.f, y}, kPlus, 14.f);
        s.text_centered("+", {top_left.x + size.x - 12.f, y}, kPlus, 14.f);
    }
    // The Gaussian surface lies inside the metal.
    s.rect({top_left.x + 38.f, top_left.y + 38.f}, {size.x - 76.f, size.y - 76.f}, kGood, 2.f, 14.f);
    s.text_centered("E = 0", top_left + size * 0.5f + glm::vec2(0.f, -8.f), kGood, 20.f);
    s.text_centered("Q = 0", top_left + size * 0.5f + glm::vec2(0.f, 20.f), kGood, 16.f);

    for (int i = 0; i < 3; ++i) {
        const float y = top_left.y + 40.f + 45.f * static_cast<float>(i);
        s.arrow({top_left.x + size.x, y}, {top_left.x + size.x + 56.f, y}, kField, 2.5f, 9.f);
        s.arrow({top_left.x, y}, {top_left.x - 56.f, y}, kField, 2.5f, 9.f);
    }
    s.label("help.fig.perpendicular", {top_left.x + size.x + 12.f, top_left.y - 10.f}, kField);
}

void draw_vector_vs_magnitude(HelpSketch& s) {
    // Left: on a sphere the length of E is the same, its direction is not.
    const glm::vec2 c{150.f, 118.f};
    s.circle(c, 54.f, kFaint, 1.5f);
    s.charge(c, true, 11.f);
    radial_arrows(s, c, 54.f, 100.f, 8, kField, rad(22.5f));
    s.label_centered("help.fig.same_magnitude", {150.f, 232.f}, kField, 15.f);
    s.label_centered("help.fig.many_directions", {150.f, 254.f});

    // Right: a patch has an area dA and a normal; the vector area is the two together.
    const glm::vec2 patch{470.f, 170.f};
    s.line(patch - glm::vec2(60.f, 0.f), patch + glm::vec2(60.f, 0.f), kMuted, 6.f);
    s.arrow(patch, patch + glm::vec2(0.f, -100.f), kGood, 3.f, 11.f);
    s.text("n", patch + glm::vec2(-24.f, -92.f), kGood, 16.f);
    s.label("help.fig.patch_area", patch + glm::vec2(-52.f, 14.f), kMuted, 15.f);
    s.text("dA = n · dA", patch + glm::vec2(14.f, -62.f), kGood, 17.f);
    s.label("help.fig.vector_area", patch + glm::vec2(14.f, -40.f));
}

}
