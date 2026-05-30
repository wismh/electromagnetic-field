#pragma once

#include <engine/core/input_system.h>

#include <game/preset_catalog.h>

#include <array>
#include <optional>

namespace game {

// No right-arrow action: single-step uses key repeat, and InputEvent is only the first press.
struct InputActions {
    engine::ActionId close{};
    engine::ActionId help{};
    engine::ActionId confirm{};
    engine::ActionId up{};
    engine::ActionId down{};
    engine::ActionId pause{};
    engine::ActionId panel{};
    engine::ActionId potential{};
    engine::ActionId lines{};
    engine::ActionId grid{};
    engine::ActionId flow{};
    engine::ActionId probe{};
    engine::ActionId trails{};
    engine::ActionId magnetic_layer{};
    engine::ActionId collisions{};
    engine::ActionId magnetic{};
    engine::ActionId reset_camera{};
    engine::ActionId clear{};
    engine::ActionId reload{};
    engine::ActionId fix_charge{};
    engine::ActionId flip_charge{};
    engine::ActionId remove_charge{};
    std::array<engine::ActionId, kPresetCatalog.size()> preset{};

    [[nodiscard]] std::optional<Preset> preset_for(engine::ActionId action) const;
};

[[nodiscard]] InputActions bind_actions(engine::InputSystem& input);

}
