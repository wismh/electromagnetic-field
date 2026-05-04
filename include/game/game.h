#pragma once

#include <engine/ecs/entity.h>
#include <engine/igame.h>
#include <engine/resources/assets_db.h>

#include <game/simulation.h>

namespace game {

class Game final : public engine::GameBase {
public:
    explicit Game(engine::AssetsDb& assets);

    engine::WindowDesc primary_window() const override;

    void on_start() override;
    void on_fixed_update() override;
    void on_update() override;

private:
    void spawn_camera();
    void spawn_potential_layer();
    void spawn_demo_charges();
    void sync_potential_uniforms();

    engine::AssetsDb& assets_;
    Simulation sim_;
    engine::ecs::Entity potential_layer_{};
};

}
