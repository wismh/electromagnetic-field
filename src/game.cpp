#include <game/game.h>

#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>

namespace game {
namespace {

constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
constexpr float kOrthoHalf = 10.f;

}

engine::WindowDesc Game::primary_window() const {
    return {.title = "Electromagnetic Field", .size = {kWindowW, kWindowH}};
}

void Game::on_start() {
    spawn_camera();
}

void Game::spawn_camera() {
    const engine::ecs::Entity camera = world_.create();
    world_.emplace<engine::Transform>(camera);
    world_.emplace<engine::Camera>(camera, engine::Camera{.ortho_size = kOrthoHalf});
    world_.ctx<engine::ActiveCamera>().entity = camera;
}

}
