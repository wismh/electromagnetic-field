#include <game/preset_catalog.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace {

constexpr std::array<game::Preset, 10> kAllPresets{
        game::Preset::Dipole,
        game::Preset::LikePair,
        game::Preset::Quadrupole,
        game::Preset::Capacitor,
        game::Preset::Orbit,
        game::Preset::Rutherford,
        game::Preset::Swarm,
        game::Preset::Cyclotron,
        game::Preset::ExBDrift,
        game::Preset::Coil,
};

}

TEST(PresetCatalog, CoversEveryPresetOnce) {
    EXPECT_EQ(game::kPresetCatalog.size(), kAllPresets.size());
    for (const game::Preset preset : kAllPresets) {
        int found = 0;
        for (const game::PresetEntry& entry : game::kPresetCatalog) {
            if (entry.preset == preset) {
                ++found;
            }
        }
        EXPECT_EQ(found, 1);
        const game::PresetEntry* entry = game::find_preset(preset);
        ASSERT_NE(entry, nullptr);
        EXPECT_EQ(game::find_preset(entry->key), entry);
        EXPECT_STREQ(game::preset_locale_key(preset), entry->locale_key);
    }
}
