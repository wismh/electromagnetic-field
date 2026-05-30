#pragma once

#include <engine/ecs/world.h>

#include <game/panel_view_model.h>
#include <game/scene_gallery.h>
#include <game/sim_clock.h>

#include <memory>

namespace game {

class Game;
class Help;

class PanelHud {
public:
    PanelHud(engine::ecs::World& world, Game& game, SimClock& clock, Help& help, SceneGallery& scenes);

    [[nodiscard]] PanelViewModel& view_model();

private:
    void toggle_pause();
    void step();
    void clear();
    void open_scenes();
    void open_help();
    void locale_en();
    void locale_uk();
    void bind_commands();

    engine::ecs::World& world_;
    Game& game_;
    SimClock& clock_;
    Help& help_;
    SceneGallery& scenes_;
    std::shared_ptr<PanelViewModel> vm_;
};

}
