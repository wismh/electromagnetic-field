#pragma once

#include <engine/igame.h>

namespace game {

class Game final : public engine::GameBase {
public:
    engine::WindowDesc primary_window() const override;

    void on_start() override;

private:
    void spawn_camera();
};

}
