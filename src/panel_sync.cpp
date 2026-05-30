#include <game/panel_sync.h>

#include <game/locale_style.h>
#include <game/magnetism.h>
#include <game/preset_catalog.h>
#include <game/slider.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

namespace game {
namespace {

// log2(scale): halving and doubling are equal distances.
constexpr SliderRange kTimeScaleLog2Range{.min = -3.f, .max = 3.f, .step = 0.25f};
constexpr SliderRange kCoulombRange{.min = 0.1f, .max = 5.f, .step = 0.05f};
constexpr SliderRange kSofteningRange{.min = 0.05f, .max = 1.f, .step = 0.01f};
constexpr SliderRange kNewChargeRange{.min = kMinAbsCharge, .max = kMaxAbsCharge, .step = kChargeStep};
constexpr SliderRange kExternalBRange{.min = -2.f, .max = 2.f, .step = 0.05f};
// log10(c) from 3 to 100. Low c exaggerates magnetism between moving charges.
constexpr SliderRange kLightSpeedLog10Range{.min = 0.48f, .max = 2.f, .step = 0.02f};

constexpr float kProbeOffsetPx = 18.f;
constexpr float kProbeBoxW = 170.f;
constexpr float kProbeBoxH = 64.f;
constexpr float kProbeBoxHMagnetic = 84.f;
constexpr float kPanelHiddenRight = -320.f;

std::string catalog_text(const engine::loc::Catalog& catalog, std::string_view key) {
    return catalog.text(key).text;
}

// Slash, not a stacked fraction: the row is one line tall.
// Leading thin space: text before \(\) is measured by ink, so a word space would not show.
std::string time_scale_text(float scale) {
    const float octaves = std::log2(scale);
    if (std::abs(octaves - std::round(octaves)) > 1e-3f) {
        return std::format("\\(\\ \\times {:.2g}\\)", scale);
    }
    if (scale < 1.f) {
        return std::format("\\(\\ \\times 1/{}\\)", static_cast<int>(std::lround(1.f / scale)));
    }
    return std::format("\\(\\ \\times {}\\)", static_cast<int>(std::lround(scale)));
}

std::string percent(float fraction) {
    return std::format("{:.1f}%", 100.f * std::clamp(fraction, 0.f, 1.f));
}

}

void PanelSync::sync(PanelViewModel& vm, Simulation& sim, FieldLayers& layers, SimClock& clock, float& new_charge,
        const PanelFrame& frame) {
    const bool pull = synced_;
    synced_ = true;
    const auto pull_toggle = [pull](const engine::ui::Bindable<bool>& field, bool echo, bool& target) {
        if (pull && field.get() != echo) {
            target = field.get();
        }
    };
    pull_toggle(vm.showPotential, echo_.layers.potential, layers.potential);
    pull_toggle(vm.showLines, echo_.layers.lines, layers.lines);
    pull_toggle(vm.showGrid, echo_.layers.grid, layers.grid);
    pull_toggle(vm.showFlow, echo_.layers.flow, layers.flow);
    pull_toggle(vm.showProbe, echo_.layers.probe, layers.probe);
    pull_toggle(vm.showTrails, echo_.layers.trails, layers.trails);
    pull_toggle(vm.showMagnetic, echo_.layers.magnetic, layers.magnetic);
    DynamicsOptions dynamics = sim.dynamics();
    pull_toggle(vm.collisions, echo_.collisions, dynamics.collisions);
    pull_toggle(vm.magnetic, echo_.magnetic, dynamics.magnetic);
    sim.set_dynamics(dynamics);

    FieldParams params = sim.params();
    if (pull && vm.timeFrac.get() != echo_.time_frac) {
        clock.set_scale(std::exp2(kTimeScaleLog2Range.from_fraction(vm.timeFrac.get())));
    }
    if (pull && vm.kFrac.get() != echo_.k_frac) {
        params.k = kCoulombRange.from_fraction(vm.kFrac.get());
    }
    if (pull && vm.epsFrac.get() != echo_.eps_frac) {
        params.softening = kSofteningRange.from_fraction(vm.epsFrac.get());
    }
    if (pull && vm.bFrac.get() != echo_.b_frac) {
        params.b_external = kExternalBRange.from_fraction(vm.bFrac.get());
    }
    if (pull && vm.cFrac.get() != echo_.c_frac) {
        params.light_speed = std::pow(10.f, kLightSpeedLog10Range.from_fraction(vm.cFrac.get()));
    }
    sim.set_params(params);
    if (pull && vm.newChargeFrac.get() != echo_.new_charge_frac) {
        new_charge = kNewChargeRange.from_fraction(vm.newChargeFrac.get());
    }

    vm.showPotential = layers.potential;
    vm.showLines = layers.lines;
    vm.showGrid = layers.grid;
    vm.showFlow = layers.flow;
    vm.showProbe = layers.probe;
    vm.showTrails = layers.trails;
    vm.showMagnetic = layers.magnetic;
    vm.collisions = dynamics.collisions;
    vm.magnetic = dynamics.magnetic;
    echo_.layers = layers;
    echo_.collisions = dynamics.collisions;
    echo_.magnetic = dynamics.magnetic;

    const float time_frac = kTimeScaleLog2Range.to_fraction(std::log2(clock.scale));
    vm.timeFrac = time_frac;
    echo_.time_frac = time_frac;
    vm.timeFill = percent(time_frac);
    const float k_frac = kCoulombRange.to_fraction(params.k);
    const float eps_frac = kSofteningRange.to_fraction(params.softening);
    const float q_frac = kNewChargeRange.to_fraction(new_charge);
    vm.kFrac = k_frac;
    vm.epsFrac = eps_frac;
    vm.newChargeFrac = q_frac;
    echo_.k_frac = k_frac;
    echo_.eps_frac = eps_frac;
    echo_.new_charge_frac = q_frac;
    vm.kText = std::format("{:.2f}", params.k);
    vm.kFill = percent(k_frac);
    vm.epsText = std::format("{:.2f}", params.softening);
    vm.epsFill = percent(eps_frac);
    vm.newChargeText = std::format("{:.2f}", new_charge);
    vm.newChargeFill = percent(q_frac);
    const float b_frac = kExternalBRange.to_fraction(params.b_external);
    vm.bFrac = b_frac;
    echo_.b_frac = b_frac;
    vm.bText = std::format("{:+.2f}", params.b_external);
    vm.bFill = percent(b_frac);
    const float c_frac = kLightSpeedLog10Range.to_fraction(std::log10(params.light_speed));
    vm.cFrac = c_frac;
    echo_.c_frac = c_frac;
    vm.cText = std::format("{:.0f}", params.light_speed);
    vm.cFill = percent(c_frac);

    const engine::loc::Catalog& catalog = *frame.catalog;
    vm.statusText = std::format("{} · {}", catalog_text(catalog, clock.paused ? "status.paused" : "status.running"),
            time_scale_text(clock.scale));
    vm.statusColor = clock.paused ? "#fcd34d" : "#a5f3c4";
    if (frame.scene_cleared) {
        vm.sceneText = catalog_text(catalog, "status.scene_empty");
    } else {
        const std::string name = catalog_text(catalog, preset_locale_key(frame.preset));
        const engine::loc::Arg name_arg{"name", std::string_view{name}};
        vm.sceneText = catalog.text("status.scene", std::span<const engine::loc::Arg>(&name_arg, 1)).text;
    }
    vm.pauseLabel = catalog_text(catalog, clock.paused ? "panel.resume" : "panel.pause");
    paint_locale_segment(
            LocaleSegment{vm.localeEnBg, vm.localeEnFg, vm.localeUkBg, vm.localeUkFg}, catalog.active() == "en");
    vm.timeScaleText = time_scale_text(clock.scale);
    vm.panelRight = std::format("{}", frame.panel_visible ? 12.f : kPanelHiddenRight);

    const float kinetic = kinetic_energy(frame.charges);
    const float potential = potential_energy(frame.charges, params);
    vm.energyKinetic = std::format("{:.3f}", kinetic);
    vm.energyPotential = std::format("{:.3f}", potential);
    vm.energyTotal = std::format("{:.3f}", kinetic + potential);
    const auto fixed_count =
            std::count_if(frame.charges.begin(), frame.charges.end(), [](const Charge& c) { return c.fixed; });
    const auto free_count = static_cast<std::ptrdiff_t>(frame.charges.size()) - fixed_count;
    vm.chargeCount = std::format("{} / {}", free_count, fixed_count);
    float fastest = 0.f;
    for (const Charge& c : frame.charges) {
        if (!c.fixed) {
            fastest = std::max(fastest, glm::length(c.velocity));
        }
    }
    vm.speedRatio = std::format("{:.2f}", params.light_speed > 0.f ? fastest / params.light_speed : 0.f);

    vm.probeVisibility = frame.show_probe ? "visible" : "hidden";
    if (frame.show_probe) {
        float x = frame.pointer_screen.x + kProbeOffsetPx;
        float y = frame.pointer_screen.y + kProbeOffsetPx;
        if (x + kProbeBoxW > static_cast<float>(frame.window.width)) {
            x = frame.pointer_screen.x - kProbeOffsetPx - kProbeBoxW;
        }
        const float probe_box_h = layers.magnetic ? kProbeBoxHMagnetic : kProbeBoxH;
        if (y + probe_box_h > static_cast<float>(frame.window.height)) {
            y = frame.pointer_screen.y - kProbeOffsetPx - probe_box_h;
        }
        vm.probeLeft = std::format("{:.0f}", x);
        vm.probeTop = std::format("{:.0f}", y);
        const glm::vec3 e = field_at(frame.charges, frame.pointer_world, params);
        vm.probeField = std::format("|E| = F/q = {:.3f}", glm::length(e));
        vm.probeComponents = std::format("E = ({:.3f},\\, {:.3f})", e.x, e.y);
        vm.probePotential = std::format("\\varphi = {:.3f}", potential_at(frame.charges, frame.pointer_world, params));
        vm.probeMagneticDisplay = layers.magnetic ? "block" : "none";
        if (layers.magnetic) {
            vm.probeMagnetic = std::format(
                    "B_z = {:.3f}", magnetic_z(frame.charges, frame.pointer_world, params, kNoCharge, frame.coils));
        }
    } else {
        vm.probeMagneticDisplay = "none";
    }
}

}
