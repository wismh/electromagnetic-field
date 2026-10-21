#pragma once

#include <game/help_sketch.h>

#include <span>

namespace game {

// A schematic figure of the reference, drawn on HelpSketch's design canvas.
using DiagramFn = void (*)(HelpSketch&);

// The figures of one topic, in the order of `diagramN` in its XML.
enum class DiagramSet {
    None,
    Coulomb,
    Field,
    Superposition,
    FieldLines,
    Gauss,
    Derivation,
    GaussPractice,
    GaussPitfalls,
    Potential,
    Voltage,
    Gradient,
    PairEnergy,
    EnergyZero,
    Bfield,
    Lorentz,
    Cyclotron,
    Exb,
    Coil,
    HistoryElectric,
    HistoryField,
    HistoryMagnetic,
};

[[nodiscard]] std::span<const DiagramFn> diagram_set(DiagramSet set);

// help_diagrams_electric.cpp
void draw_coulomb_forces(HelpSketch& s);
void draw_inverse_square(HelpSketch& s);
void draw_field_around_charge(HelpSketch& s);
void draw_superposition(HelpSketch& s);
void draw_line_rules(HelpSketch& s);
void draw_gauss_cases(HelpSketch& s);
void draw_action_at_distance(HelpSketch& s);

// help_diagrams_gauss.cpp
void draw_flux_angle(HelpSketch& s);
void draw_sphere_flux(HelpSketch& s);
void draw_sphere_area(HelpSketch& s);
void draw_solid_angle(HelpSketch& s);
void draw_outside_charge(HelpSketch& s);
void draw_flux_superposition(HelpSketch& s);
void draw_gauss_cylinder(HelpSketch& s);
void draw_gauss_pillbox(HelpSketch& s);
void draw_gauss_conductor(HelpSketch& s);
void draw_vector_vs_magnitude(HelpSketch& s);

// help_diagrams_energy.cpp
void draw_equipotentials(HelpSketch& s);
void draw_capacitor(HelpSketch& s);
void draw_potential_slope(HelpSketch& s);
void draw_pair_energy_curves(HelpSketch& s);
void draw_energy_zero_levels(HelpSketch& s);

// help_diagrams_magnetic.cpp
void draw_wire_field(HelpSketch& s);
void draw_lorentz_directions(HelpSketch& s);
void draw_cyclotron_orbit(HelpSketch& s);
void draw_exb_cycloid(HelpSketch& s);
void draw_coil_field(HelpSketch& s);

// help_diagrams_history.cpp
void draw_torsion_balance(HelpSketch& s);
void draw_cavendish_spheres(HelpSketch& s);
void draw_oersted_needle(HelpSketch& s);

}
