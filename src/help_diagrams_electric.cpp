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

float length(glm::vec2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

glm::vec2 normalized(glm::vec2 v) {
    return v / length(v);
}

}

void draw_coulomb_forces(HelpSketch& s) {
    const float left = 200.f;
    const float right = 440.f;
    // Like charges push apart, unlike ones pull together; the two forces are equal and opposite.
    const struct {
        float y;
        bool second_positive;
        const char* caption;
    } rows[] = {{74.f, true, "help.fig.repel"}, {214.f, false, "help.fig.attract"}};
    for (const auto& row : rows) {
        const glm::vec2 a{left, row.y};
        const glm::vec2 b{right, row.y};
        s.charge(a, true, 15.f);
        s.charge(b, row.second_positive, 15.f);
        const float dir = row.second_positive ? 1.f : -1.f;
        s.arrow(a + glm::vec2(-dir * 20.f, 0.f), a + glm::vec2(-dir * 92.f, 0.f), kForce, 3.f, 11.f);
        s.arrow(b + glm::vec2(dir * 20.f, 0.f), b + glm::vec2(dir * 92.f, 0.f), kForce, 3.f, 11.f);
        s.text_centered("F1", a + glm::vec2(-dir * 56.f, -22.f), kForce, 15.f);
        s.text_centered("F2", b + glm::vec2(dir * 56.f, -22.f), kForce, 15.f);
        s.text_centered("q1", a + glm::vec2(0.f, -34.f), kText, 14.f);
        s.text_centered("q2", b + glm::vec2(0.f, -34.f), kText, 14.f);
        s.dashed(a + glm::vec2(22.f, 34.f), b + glm::vec2(-22.f, 34.f), kMuted, 1.2f, 5.f);
        s.text_centered("r", {(left + right) * 0.5f, row.y + 48.f}, kText, 15.f);
        s.label_centered(row.caption, {(left + right) * 0.5f, row.y - 58.f}, kMuted, 14.f);
    }
}

void draw_inverse_square(HelpSketch& s) {
    const glm::vec2 c{60.f, 110.f};
    s.charge(c, true, 13.f);
    // Squares at r, 2r and 3r, and the force bar of the same distance below.
    const float distances[] = {110.f, 220.f, 330.f};
    const char* sizes[] = {"S", "4S", "9S"};
    const char* names[] = {"r", "2r", "3r"};
    const char* forces[] = {"F", "F/4", "F/9"};
    const float bars[] = {64.f, 16.f, 7.1f};
    constexpr float kHalf = 0.16f;
    s.dashed(c, c + glm::vec2(430.f, -430.f * std::tan(kHalf)), kMuted, 1.5f, 8.f);
    s.dashed(c, c + glm::vec2(430.f, 430.f * std::tan(kHalf)), kMuted, 1.5f, 8.f);
    for (int i = 0; i < 3; ++i) {
        const float x = c.x + distances[i];
        const float h = distances[i] * std::tan(kHalf);
        s.rect({x - h, c.y - h}, {2.f * h, 2.f * h}, kField, 2.f);
        s.text_centered(sizes[i], {x, c.y}, kField, 16.f);
        s.text_centered(names[i], {x, c.y + h + 22.f}, kText, 15.f);
        // The force at the same distance.
        s.fill({x - 15.f, 258.f - bars[i]}, {30.f, bars[i]}, kForce, 3.f);
        s.text_centered(forces[i], {x, 274.f}, kForce, 15.f);
    }
    s.line({c.x + 50.f, 260.f}, {c.x + distances[2] + 70.f, 260.f}, kFaint, 1.5f);
}

void draw_field_around_charge(HelpSketch& s) {
    const glm::vec2 c{230.f, 140.f};
    s.charge(c, true, 14.f);
    // Arrows get shorter as 1/r^2: the length is 48, 12 and 5.4 at distances 30, 60 and 90.
    const struct {
        float distance;
        float length;
    } rings[] = {{34.f, 52.f}, {82.f, 13.f}, {128.f, 5.5f}};
    for (const auto& ring : rings) {
        const int count = ring.distance < 50.f ? 8 : 12;
        for (int i = 0; i < count; ++i) {
            const glm::vec2 d = HelpSketch::direction(2.f * kPi * static_cast<float>(i) / static_cast<float>(count));
            s.arrow(c + d * ring.distance, c + d * (ring.distance + ring.length), kField, ring.length > 20.f ? 2.5f : 2.f,
                    ring.length > 20.f ? 10.f : 6.f);
        }
    }
    s.text("E", c + glm::vec2(58.f, -18.f), kField, 17.f);

    // A test charge and the force on it.
    const glm::vec2 q{480.f, 140.f};
    s.charge(q, true, 9.f);
    s.arrow(q + glm::vec2(12.f, 0.f), q + glm::vec2(88.f, 0.f), kForce, 3.f, 11.f);
    s.text("F", q + glm::vec2(46.f, -26.f), kForce, 16.f);
    s.text("q", q + glm::vec2(-6.f, -34.f), kText, 15.f);
    s.text("E = F / q", q + glm::vec2(-22.f, 38.f), kWhite, 17.f);
}

void draw_superposition(HelpSketch& s) {
    const glm::vec2 q1{110.f, 50.f};
    const glm::vec2 q2{180.f, 220.f};
    const glm::vec2 p{420.f, 130.f};
    s.dashed(q1, p, kFaint, 1.4f, 6.f);
    s.dashed(q2, p, kFaint, 1.4f, 6.f);
    s.charge(q1, true, 14.f);
    s.charge(q2, false, 14.f);
    s.text("Q1", q1 + glm::vec2(-8.f, -42.f), kText, 15.f);
    s.text("Q2", q2 + glm::vec2(-8.f, 20.f), kText, 15.f);
    s.disc(p, 5.f, kWhite);
    s.text("P", p + glm::vec2(8.f, 6.f), kText, 15.f);

    const glm::vec2 e1 = normalized(p - q1) * 110.f;
    const glm::vec2 e2 = normalized(q2 - p) * 130.f;
    const glm::vec2 sum = e1 + e2;
    s.dashed(p + e1, p + sum, kMuted, 1.4f, 5.f);
    s.dashed(p + e2, p + sum, kMuted, 1.4f, 5.f);
    s.arrow(p, p + e1, kField, 3.f, 11.f);
    s.arrow(p, p + e2, kField, 3.f, 11.f);
    s.arrow(p, p + sum, kGood, 4.f, 13.f);
    s.text("E1", p + e1 + glm::vec2(8.f, -6.f), kField, 16.f);
    s.text("E2", p + e2 + glm::vec2(-26.f, 4.f), kField, 16.f);
    s.text("E", p + sum + glm::vec2(-26.f, 4.f), kGood, 18.f);
    s.text("E = E1 + E2", {440.f, 28.f}, kWhite, 17.f);
}

void draw_line_rules(HelpSketch& s) {
    // Lines start on +, end on −, and never cross.
    const glm::vec2 plus{120.f, 120.f};
    const glm::vec2 minus{320.f, 120.f};
    for (int i = 0; i < 8; ++i) {
        const glm::vec2 d = HelpSketch::direction(rad(22.5f + 45.f * static_cast<float>(i)));
        s.arrow(plus + d * 18.f, plus + d * 84.f, kField, 2.5f, 10.f);
        s.arrow(minus + d * 84.f, minus + d * 18.f, kField, 2.5f, 10.f);
    }
    s.charge(plus, true, 14.f);
    s.charge(minus, false, 14.f);

    // Two lines cannot cross: the field would have two directions at one point.
    const glm::vec2 x{520.f, 120.f};
    s.arrow(x + glm::vec2(-70.f, 50.f), x + glm::vec2(70.f, -50.f), kField, 2.5f, 10.f);
    s.arrow(x + glm::vec2(-70.f, -50.f), x + glm::vec2(70.f, 50.f), kField, 2.5f, 10.f);
    s.disc(x, 5.f, kBad);
    s.line(x + glm::vec2(-14.f, -76.f) + glm::vec2(0.f, -10.f), x + glm::vec2(14.f, -50.f) + glm::vec2(0.f, -10.f), kBad, 3.f);
    s.line(x + glm::vec2(-14.f, -50.f) + glm::vec2(0.f, -10.f), x + glm::vec2(14.f, -76.f) + glm::vec2(0.f, -10.f), kBad, 3.f);
    s.label_centered("help.fig.lines_start", {120.f, 238.f});
    s.label_centered("help.fig.lines_end", {320.f, 238.f});
    s.label_centered("help.fig.lines_cross", {520.f, 238.f});
}

void draw_gauss_cases(HelpSketch& s) {
    constexpr float kRadius = 66.f;
    const float y = 118.f;
    const float xs[] = {110.f, 320.f, 530.f};

    // A charge inside: every line goes out.
    {
        const glm::vec2 c{xs[0], y};
        s.circle(c, kRadius, kText, 2.5f);
        for (int i = 0; i < 8; ++i) {
            const glm::vec2 d = HelpSketch::direction(rad(22.5f + 45.f * static_cast<float>(i)));
            s.arrow(c + d * 16.f, c + d * (kRadius + 26.f), kGood, 2.f, 9.f);
        }
        s.charge(c, true, 12.f);
        s.text_centered("Φ > 0", {c.x, 232.f}, kGood, 18.f);
    }
    // A charge outside: the lines go in on one side and out on the other.
    {
        const glm::vec2 c{xs[1], y};
        const glm::vec2 source{c.x - 128.f, y};
        s.circle(c, kRadius, kText, 2.5f);
        for (int i = -2; i <= 2; ++i) {
            const glm::vec2 d = normalized(glm::vec2(1.f, 0.30f * static_cast<float>(i)));
            // Where the line meets the circle: the entry (red) and the exit (green).
            const glm::vec2 dir = d;
            const glm::vec2 oc = source - c;
            const float b = oc.x * dir.x + oc.y * dir.y;
            const float disc_value = b * b - (oc.x * oc.x + oc.y * oc.y - kRadius * kRadius);
            if (disc_value <= 0.f) {
                s.dashed(source, source + dir * 250.f, kFaint, 1.2f, 5.f);
                continue;
            }
            const float root = std::sqrt(disc_value);
            const float enter = -b - root;
            const float leave = -b + root;
            s.line(source + dir * 14.f, source + dir * enter, kBad, 2.f);
            s.arrow(source + dir * (enter - 16.f), source + dir * enter, kBad, 2.f, 9.f);
            s.line(source + dir * enter, source + dir * leave, kMuted, 1.5f);
            s.arrow(source + dir * leave, source + dir * (leave + 26.f), kGood, 2.f, 9.f);
        }
        s.charge(source, true, 12.f);
        s.text_centered("Φ = 0", {c.x, 232.f}, kText, 18.f);
    }
    // A dipole inside: lines that leave come back.
    {
        const glm::vec2 c{xs[2], y};
        s.circle(c, kRadius, kText, 2.5f);
        const glm::vec2 plus{c.x - 20.f, y};
        const glm::vec2 minus{c.x + 20.f, y};
        s.arc(c, 20.f, 0.f, kPi, kField, 2.f);
        s.arc(c, 20.f, kPi, 2.f * kPi, kField, 2.f);
        // One line leaves from the + and returns to the −, outside the surface.
        s.arrow(plus + glm::vec2(-10.f, -8.f), c + glm::vec2(-kRadius - 22.f, -34.f), kGood, 2.f, 9.f);
        s.arrow(c + glm::vec2(kRadius + 22.f, -34.f), minus + glm::vec2(10.f, -8.f), kBad, 2.f, 9.f);
        s.dashed(c + glm::vec2(-kRadius - 22.f, -34.f), c + glm::vec2(kRadius + 22.f, -34.f), kFaint, 1.2f, 5.f);
        s.charge(plus, true, 11.f);
        s.charge(minus, false, 11.f);
        s.text_centered("Φ = 0", {c.x, 232.f}, kText, 18.f);
    }
}

void draw_action_at_distance(HelpSketch& s) {
    // Left: two charges and a force straight between them, as in Coulomb's picture.
    const glm::vec2 a{70.f, 115.f};
    const glm::vec2 b{250.f, 115.f};
    s.charge(a, true, 14.f);
    s.charge(b, false, 14.f);
    s.arrow(a + glm::vec2(20.f, -6.f), b + glm::vec2(-20.f, -6.f), kForce, 3.f, 12.f);
    s.arrow(b + glm::vec2(-20.f, 6.f), a + glm::vec2(20.f, 6.f), kForce, 3.f, 12.f);
    s.text_centered("F", {160.f, 82.f}, kForce, 17.f);
    s.label_centered("help.fig.action_distance", {160.f, 190.f}, kText, 15.f);

    // Right: the first charge fills the space with a field; the second one meets it where it sits.
    const glm::vec2 c{380.f, 115.f};
    const glm::vec2 d{580.f, 115.f};
    for (int i = 0; i < 12; ++i) {
        const glm::vec2 dir = HelpSketch::direction(2.f * kPi * static_cast<float>(i) / 12.f + 0.26f);
        s.arrow(c + dir * 20.f, c + dir * (i % 3 == 0 ? 78.f : 60.f), kField, 2.f, 8.f);
    }
    s.charge(c, true, 14.f);
    s.charge(d, false, 14.f);
    s.arrow(d + glm::vec2(-20.f, 0.f), d + glm::vec2(-72.f, 0.f), kField, 3.f, 11.f);
    s.arrow(d + glm::vec2(-20.f, 14.f), d + glm::vec2(-72.f, 14.f), kForce, 3.f, 11.f);
    s.text("E", d + glm::vec2(-52.f, -26.f), kField, 16.f);
    s.text("F = qE", d + glm::vec2(-72.f, 32.f), kForce, 16.f);
    s.label_centered("help.fig.through_field", {480.f, 190.f}, kText, 15.f);
    s.line({320.f, 30.f}, {320.f, 210.f}, kFaint, 1.5f);
}

}
