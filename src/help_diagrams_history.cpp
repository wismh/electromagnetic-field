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

}

void draw_torsion_balance(HelpSketch& s) {
    // Top view of Coulomb’s torsion balance. The wire hangs from the centre; a light rod carries a ball a and a
    // counterweight. The fixed ball b pushes a away and the rod turns by an angle θ, twisting the wire.
    const glm::vec2 c{190.f, 130.f};
    constexpr float kCase = 108.f;
    constexpr float kArm = 84.f;
    s.circle(c, kCase, kMuted, 2.f);
    s.dashed(c, c + HelpSketch::direction(rad(0.f)) * kCase, kFaint, 1.5f, 6.f);
    const float turn = 38.f;
    const glm::vec2 a = c + HelpSketch::direction(rad(turn)) * kArm;
    const glm::vec2 weight = c - HelpSketch::direction(rad(turn)) * kArm;
    s.line(weight, a, kText, 3.f);
    s.disc(weight, 7.f, kMuted);
    s.disc(a, 11.f, kPlus);
    s.arc(c, 52.f, 0.f, rad(turn), kForce, 2.f);
    s.text("θ", c + HelpSketch::direction(rad(turn * 0.5f)) * 68.f - glm::vec2(5.f, 10.f), kForce, 17.f);
    const glm::vec2 b = c + HelpSketch::direction(rad(turn + 28.f)) * kArm;
    s.disc(b, 11.f, kPlus);
    s.disc(c, 4.f, kWhite);
    s.text("a", a + glm::vec2(-26.f, 0.f), kText, 17.f);
    s.text("b", b + glm::vec2(-6.f, -34.f), kText, 17.f);
    s.arrow(a + HelpSketch::direction(rad(turn - 90.f)) * 18.f, a + HelpSketch::direction(rad(turn - 90.f)) * 54.f, kGood, 2.5f,
            10.f);

    // Side view: the wire, the rod and the balls under a glass cover.
    const float x = 440.f;
    s.rect({x - 100.f, 52.f}, {200.f, 190.f}, kMuted, 2.f, 6.f);
    s.fill({x - 12.f, 28.f}, {24.f, 22.f}, kMuted, 4.f);
    s.line({x, 50.f}, {x, 130.f}, kWhite, 1.4f);
    s.line({x - 76.f, 130.f}, {x + 76.f, 130.f}, kText, 3.f);
    s.disc({x + 76.f, 130.f}, 10.f, kPlus);
    s.disc({x - 76.f, 130.f}, 6.f, kMuted);
    s.line({x + 54.f, 52.f}, {x + 54.f, 132.f}, kMuted, 2.f);
    s.disc({x + 54.f, 134.f}, 9.f, kPlus);
    s.label("help.fig.micrometer", {x - 40.f, 6.f}, kMuted, 14.f);
    s.text("F = κ θ", {x - 30.f, 258.f}, kForce, 18.f);
}

void draw_cavendish_spheres(HelpSketch& s) {
    // Charge a hollow shell with a smaller sphere inside, joined to it by a wire. Then take the wire away and test
    // the inner sphere. With a 1/r² force it holds no charge at all.
    const struct {
        float x;
        bool joined;
    } panels[] = {{150.f, true}, {450.f, false}};
    for (const auto& panel : panels) {
        const glm::vec2 c{panel.x, 130.f};
        s.circle(c, 100.f, kText, 3.f);
        s.circle(c, 48.f, kText, 2.f);
        for (int i = 0; i < 16; ++i) {
            const glm::vec2 d = HelpSketch::direction(2.f * kPi * static_cast<float>(i) / 16.f);
            s.text_centered("+", c + d * 112.f, kPlus, 15.f);
        }
        if (panel.joined) {
            s.line(c + glm::vec2(0.f, -48.f), c + glm::vec2(0.f, -100.f), kForce, 2.5f);
            s.line(c + glm::vec2(0.f, -100.f), c + glm::vec2(0.f, -122.f), kForce, 2.5f);
        } else {
            s.text_centered("q = 0", c, kGood, 20.f);
            s.line(c + glm::vec2(10.f, -105.f), c + glm::vec2(34.f, -145.f), kForce, 2.5f);
        }
    }
    s.label_centered("help.fig.cavendish_joined", {150.f, 258.f}, kText, 15.f);
    s.label_centered("help.fig.cavendish_apart", {450.f, 258.f}, kText, 15.f);
    s.text_centered("n = 2", {300.f, 130.f}, kWhite, 20.f);
}

void draw_oersted_needle(HelpSketch& s) {
    // Top view: a wire with the current to the right, and a compass needle just below it. The needle first points
    // along the wire; with the current on it turns toward the field of the wire, which is across the wire.
    const float wire = 70.f;
    s.line({40.f, wire}, {600.f, wire}, kForce, 6.f);
    s.arrow({280.f, wire - 22.f}, {400.f, wire - 22.f}, kForce, 3.f, 12.f);
    s.text("I", {408.f, wire - 36.f}, kForce, 18.f);

    const glm::vec2 c{320.f, 170.f};
    s.circle(c, 56.f, kMuted, 2.f);
    // The needle before: a faint ghost along the wire.
    const glm::vec2 ghost = HelpSketch::direction(0.f) * 46.f;
    s.line(c - ghost, c + ghost, kFaint, 4.f);
    // The needle after: turned toward B of the wire.
    const glm::vec2 north = HelpSketch::direction(rad(50.f)) * 46.f;
    s.line(c, c + north, kBad, 6.f);
    s.line(c, c - north, kMinus, 6.f);
    s.disc(c, 4.f, kWhite);
    s.arc(c, 72.f, 0.f, rad(50.f), kText, 1.8f);
    s.text("α", c + HelpSketch::direction(rad(25.f)) * 90.f - glm::vec2(4.f, 10.f), kText, 17.f);

    s.arrow(c + glm::vec2(-150.f, 20.f), c + glm::vec2(-90.f, 20.f), kMuted, 2.5f, 10.f);
    s.label("help.fig.b_earth", c + glm::vec2(-200.f, 28.f), kMuted, 15.f);
    s.arrow(c + glm::vec2(120.f, 20.f), c + glm::vec2(120.f, -30.f), kField, 2.5f, 10.f);
    s.label("help.fig.b_wire", c + glm::vec2(130.f, -14.f), kField, 15.f);
}

}
