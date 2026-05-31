#pragma once

#include <engine/core/key_code.h>

#include <game/scene.h>

#include <array>

namespace game {

struct PresetEntry {
    Preset preset;
    engine::KeyCode key;
    const char* locale_key;
};

inline constexpr std::array<PresetEntry, 10> kPresetCatalog{{
        {Preset::Dipole, engine::KeyCode::Digit1, "preset.dipole"},
        {Preset::LikePair, engine::KeyCode::Digit2, "preset.like_pair"},
        {Preset::Quadrupole, engine::KeyCode::Digit3, "preset.quadrupole"},
        {Preset::Capacitor, engine::KeyCode::Digit4, "preset.capacitor"},
        {Preset::Orbit, engine::KeyCode::Digit5, "preset.orbit"},
        {Preset::Rutherford, engine::KeyCode::Digit6, "preset.rutherford"},
        {Preset::Swarm, engine::KeyCode::Digit7, "preset.swarm"},
        {Preset::Cyclotron, engine::KeyCode::Digit8, "preset.cyclotron"},
        {Preset::ExBDrift, engine::KeyCode::Digit9, "preset.exb_drift"},
        {Preset::Coil, engine::KeyCode::Digit0, "preset.coil"},
}};

[[nodiscard]] inline const PresetEntry* find_preset(engine::KeyCode key) {
    for (const PresetEntry& entry : kPresetCatalog) {
        if (entry.key == key) {
            return &entry;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const PresetEntry* find_preset(Preset preset) {
    for (const PresetEntry& entry : kPresetCatalog) {
        if (entry.preset == preset) {
            return &entry;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const char* preset_locale_key(Preset preset) {
    const PresetEntry* entry = find_preset(preset);
    return entry != nullptr ? entry->locale_key : "preset.dipole";
}

}
