#include <game/game.h>

#include <engine/builtin_ids.h>
#include <engine/core/time.h>
#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>
#include <engine/render/renderable.h>

#include <asset_ids.h>

#include <algorithm>
#include <cstddef>
#include <string>

namespace game {
namespace {

constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
constexpr float kOrthoHalf = 10.f;
// Must match kMaxCharges in assets/shaders/potential.shader.
constexpr std::size_t kMaxShaderCharges = 32;
// Large enough to cover the view at any window aspect.
constexpr float kPotentialLayerSize = 200.f;

}

Game::Game(engine::AssetsDb& assets) :
    assets_(assets) {}

engine::WindowDesc Game::primary_window() const {
    return {.title = "Electromagnetic Field", .size = {kWindowW, kWindowH}};
}

void Game::on_start() {
    spawn_camera();
    spawn_potential_layer();
    spawn_demo_charges();
}

void Game::on_fixed_update() {
    sim_.step(engine::kFixed);
    GameBase::on_fixed_update();
}

void Game::on_update() {
    sync_potential_uniforms();
    GameBase::on_update();
}

void Game::spawn_camera() {
    const engine::ecs::Entity camera = world_.create();
    world_.emplace<engine::Transform>(camera);
    world_.emplace<engine::Camera>(camera, engine::Camera{.ortho_size = kOrthoHalf});
    world_.ctx<engine::ActiveCamera>().entity = camera;
}

void Game::spawn_potential_layer() {
    potential_layer_ = world_.create();
    world_.emplace<engine::Transform>(potential_layer_, engine::Transform{
            .scale = {kPotentialLayerSize, kPotentialLayerSize, 1.f},
    });
    world_.emplace<engine::render::Renderable>(potential_layer_, engine::render::Renderable{
            .mesh = assets_.get<engine::render::IMesh>(engine::builtin::mesh_quad),
            .material = assets_.get<engine::render::IMaterial>(assets::materials::potential),
            .layer = -100,
            .material_override = engine::render::MaterialOverride{},
    });
}

// Probe scene: two fixed charges and one free charge orbiting the positive one.
void Game::spawn_demo_charges() {
    auto& charges = sim_.charges();
    charges.push_back(Charge{.position = {-4.f, 0.f, 0.f}, .q = 2.f, .fixed = true});
    charges.push_back(Charge{.position = {4.f, 0.f, 0.f}, .q = -1.f, .fixed = true});
    charges.push_back(Charge{.position = {-4.f, 2.f, 0.f}, .velocity = {0.707f, 0.f, 0.f}, .q = -0.5f});
}

void Game::sync_potential_uniforms() {
    auto& renderable = world_.get<engine::render::Renderable>(potential_layer_);
    engine::render::MaterialOverride& override = *renderable.material_override;
    const auto& charges = sim_.charges();
    const std::size_t count = std::min(charges.size(), kMaxShaderCharges);
    for (std::size_t i = 0; i < count; ++i) {
        const Charge& c = charges[i];
        override.set_vec4("uCharges[" + std::to_string(i) + "]", {c.position.x, c.position.y, c.q, 0.f});
    }
    const FieldParams& params = sim_.params();
    override.set_vec4("uParams", {static_cast<float>(count), params.k, params.softening, 0.f});
}

}
