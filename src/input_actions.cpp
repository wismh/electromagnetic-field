#include <game/input_actions.h>

#include <cstddef>
#include <string_view>

namespace game {
namespace {

engine::ActionId bind_key(engine::InputSystem& input, engine::KeyCode key, std::string_view name) {
    const engine::ActionId action = input.intern(name);
    input.bind(key, action);
    return action;
}

}

std::optional<Preset> InputActions::preset_for(engine::ActionId action) const {
    if (action == engine::ActionId::Invalid) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < preset.size(); ++i) {
        if (preset[i] == action) {
            return kPresetCatalog[i].preset;
        }
    }
    return std::nullopt;
}

InputActions bind_actions(engine::InputSystem& input) {
    InputActions actions;
    actions.close = bind_key(input, engine::KeyCode::Escape, "close");
    actions.help = bind_key(input, engine::KeyCode::H, "help");
    actions.confirm = bind_key(input, engine::KeyCode::Return, "confirm");
    actions.up = bind_key(input, engine::KeyCode::Up, "up");
    actions.down = bind_key(input, engine::KeyCode::Down, "down");
    actions.pause = bind_key(input, engine::KeyCode::Space, "pause");
    actions.panel = bind_key(input, engine::KeyCode::Tab, "panel");
    actions.potential = bind_key(input, engine::KeyCode::F1, "layer.potential");
    actions.lines = bind_key(input, engine::KeyCode::F2, "layer.lines");
    actions.grid = bind_key(input, engine::KeyCode::F3, "layer.grid");
    actions.flow = bind_key(input, engine::KeyCode::F4, "layer.flow");
    actions.probe = bind_key(input, engine::KeyCode::F5, "layer.probe");
    actions.trails = bind_key(input, engine::KeyCode::F6, "layer.trails");
    actions.magnetic_layer = bind_key(input, engine::KeyCode::F7, "layer.magnetic");
    actions.collisions = bind_key(input, engine::KeyCode::K, "collisions");
    actions.magnetic = bind_key(input, engine::KeyCode::M, "magnetic");
    actions.reset_camera = bind_key(input, engine::KeyCode::Home, "reset_camera");
    actions.clear = bind_key(input, engine::KeyCode::C, "clear");
    actions.reload = bind_key(input, engine::KeyCode::R, "reload");
    actions.fix_charge = bind_key(input, engine::KeyCode::L, "fix_charge");
    actions.flip_charge = bind_key(input, engine::KeyCode::F, "flip_charge");
    actions.remove_charge = input.intern("remove_charge");
    input.bind(engine::KeyCode::Delete, actions.remove_charge);
    input.bind(engine::KeyCode::Backspace, actions.remove_charge);
    for (std::size_t i = 0; i < kPresetCatalog.size(); ++i) {
        actions.preset[i] = bind_key(input, kPresetCatalog[i].key, kPresetCatalog[i].locale_key);
    }
    return actions;
}

}
