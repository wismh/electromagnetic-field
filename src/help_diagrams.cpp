#include <game/help_diagrams.h>

#include <array>

namespace game {
namespace {

constexpr std::array<DiagramFn, 2> kCoulomb{draw_coulomb_forces, draw_inverse_square};
constexpr std::array<DiagramFn, 1> kField{draw_field_around_charge};
constexpr std::array<DiagramFn, 1> kSuperposition{draw_superposition};
constexpr std::array<DiagramFn, 1> kFieldLines{draw_line_rules};
constexpr std::array<DiagramFn, 1> kGauss{draw_gauss_cases};
constexpr std::array<DiagramFn, 6> kDerivation{draw_flux_angle, draw_sphere_flux, draw_sphere_area, draw_solid_angle,
        draw_outside_charge, draw_flux_superposition};
constexpr std::array<DiagramFn, 3> kGaussPractice{draw_gauss_cylinder, draw_gauss_pillbox, draw_gauss_conductor};
constexpr std::array<DiagramFn, 1> kGaussPitfalls{draw_vector_vs_magnitude};
constexpr std::array<DiagramFn, 1> kPotential{draw_equipotentials};
constexpr std::array<DiagramFn, 1> kVoltage{draw_capacitor};
constexpr std::array<DiagramFn, 1> kGradient{draw_potential_slope};
constexpr std::array<DiagramFn, 1> kPairEnergy{draw_pair_energy_curves};
constexpr std::array<DiagramFn, 1> kEnergyZero{draw_energy_zero_levels};
constexpr std::array<DiagramFn, 1> kBfield{draw_wire_field};
constexpr std::array<DiagramFn, 1> kLorentz{draw_lorentz_directions};
constexpr std::array<DiagramFn, 1> kCyclotron{draw_cyclotron_orbit};
constexpr std::array<DiagramFn, 1> kExb{draw_exb_cycloid};
constexpr std::array<DiagramFn, 1> kCoil{draw_coil_field};
constexpr std::array<DiagramFn, 2> kHistoryElectric{draw_torsion_balance, draw_cavendish_spheres};
constexpr std::array<DiagramFn, 1> kHistoryField{draw_action_at_distance};
constexpr std::array<DiagramFn, 1> kHistoryMagnetic{draw_oersted_needle};

}

std::span<const DiagramFn> diagram_set(DiagramSet set) {
    switch (set) {
        case DiagramSet::Coulomb:
            return kCoulomb;
        case DiagramSet::Field:
            return kField;
        case DiagramSet::Superposition:
            return kSuperposition;
        case DiagramSet::FieldLines:
            return kFieldLines;
        case DiagramSet::Gauss:
            return kGauss;
        case DiagramSet::Derivation:
            return kDerivation;
        case DiagramSet::GaussPractice:
            return kGaussPractice;
        case DiagramSet::GaussPitfalls:
            return kGaussPitfalls;
        case DiagramSet::Potential:
            return kPotential;
        case DiagramSet::Voltage:
            return kVoltage;
        case DiagramSet::Gradient:
            return kGradient;
        case DiagramSet::PairEnergy:
            return kPairEnergy;
        case DiagramSet::EnergyZero:
            return kEnergyZero;
        case DiagramSet::Bfield:
            return kBfield;
        case DiagramSet::Lorentz:
            return kLorentz;
        case DiagramSet::Cyclotron:
            return kCyclotron;
        case DiagramSet::Exb:
            return kExb;
        case DiagramSet::Coil:
            return kCoil;
        case DiagramSet::HistoryElectric:
            return kHistoryElectric;
        case DiagramSet::HistoryField:
            return kHistoryField;
        case DiagramSet::HistoryMagnetic:
            return kHistoryMagnetic;
        case DiagramSet::None:
            return {};
    }
    return {};
}

}
