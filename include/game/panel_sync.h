#pragma once

#include <engine/loc/catalog.h>
#include <engine/ui/canvas.h>

#include <game/field_layers.h>
#include <game/panel_view_model.h>
#include <game/scene.h>
#include <game/sim_clock.h>
#include <game/simulation.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <span>

namespace game {

// Two-way fields are sync() arguments: a difference from the last write is user input.
struct PanelFrame {
    const engine::loc::Catalog* catalog = nullptr;
    std::span<const Charge> charges;
    std::span<const Coil> coils;
    Preset preset = Preset::Dipole;
    bool scene_cleared = false;
    bool panel_visible = true;
    bool show_probe = false;
    glm::vec2 pointer_screen{0.f};
    glm::vec3 pointer_world{0.f};
    engine::ui::WindowSize window{};
};

class PanelSync {
public:
    void sync(PanelViewModel& vm, Simulation& sim, FieldLayers& layers, SimClock& clock, float& new_charge,
            const PanelFrame& frame);

private:
    struct Echo {
        FieldLayers layers;
        bool collisions = true;
        bool magnetic = false;
        float time_frac = -1.f;
        float k_frac = -1.f;
        float c_frac = -1.f;
        float eps_frac = -1.f;
        float new_charge_frac = -1.f;
        float b_frac = -1.f;
    };

    Echo echo_{};
    // False until the first write; default-constructed fields are not user input.
    bool synced_ = false;
};

}
