#include <game/scene_gallery.h>

#include <game/overlay_canvas.h>

#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <utility>

namespace game {
namespace {

constexpr int kGalleryCanvasOrder = 25;  // above the reference (20), below the welcome screen (30)
constexpr int kWindowMargin = 28;
constexpr int kMaxWindowWidth = 992;

}

SceneGalleryViewModel::SceneGalleryViewModel() {
    assets::ui::Scenes::bind(*this);
}

SceneGallery::SceneGallery(engine::ecs::World& world, ChooseHandler on_choose) :
    world_(world),
    vm_(std::make_shared<SceneGalleryViewModel>()),
    on_choose_(std::move(on_choose)) {
    vm_->closeGallery = [this] { close(); };
    vm_->presetDipole = [this] { choose(Preset::Dipole); };
    vm_->presetLikePair = [this] { choose(Preset::LikePair); };
    vm_->presetQuadrupole = [this] { choose(Preset::Quadrupole); };
    vm_->presetCapacitor = [this] { choose(Preset::Capacitor); };
    vm_->presetOrbit = [this] { choose(Preset::Orbit); };
    vm_->presetRutherford = [this] { choose(Preset::Rutherford); };
    vm_->presetSwarm = [this] { choose(Preset::Swarm); };
    vm_->presetCyclotron = [this] { choose(Preset::Cyclotron); };
    vm_->presetExBDrift = [this] { choose(Preset::ExBDrift); };
    vm_->presetCoil = [this] { choose(Preset::Coil); };

    canvas_ = world.create();
    world.emplace<engine::ui::UiCanvas>(canvas_, engine::ui::UiCanvas{
            .document = assets::ui::scenes,
            .stylesheet = assets::css::scenes,
            .data_context = vm_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = kGalleryCanvasOrder,
    });
    sync();
}

void SceneGallery::open() {
    if (pop_.open()) {
        sync();
    }
}

void SceneGallery::close() {
    if (pop_.close()) {
        sync();
    }
}

void SceneGallery::tick(float dt) {
    if (pop_.tick(dt)) {
        sync();
    }
}

void SceneGallery::choose(Preset preset) {
    close();
    if (on_choose_) {
        on_choose_(preset);
    }
}

void SceneGallery::set_current(std::optional<Preset> preset) {
    vm_->sceneDipole = preset == Preset::Dipole;
    vm_->sceneLikePair = preset == Preset::LikePair;
    vm_->sceneQuadrupole = preset == Preset::Quadrupole;
    vm_->sceneCapacitor = preset == Preset::Capacitor;
    vm_->sceneOrbit = preset == Preset::Orbit;
    vm_->sceneRutherford = preset == Preset::Rutherford;
    vm_->sceneSwarm = preset == Preset::Swarm;
    vm_->sceneCyclotron = preset == Preset::Cyclotron;
    vm_->sceneExBDrift = preset == Preset::ExBDrift;
    vm_->sceneCoil = preset == Preset::Coil;
}

void SceneGallery::update_layout(glm::ivec2 window_size) {
    const PlacedWindow placed = place_window(window_size, kWindowMargin, kMaxWindowWidth);
    vm_->windowLeft = std::to_string(placed.left);
    vm_->windowWidth = std::to_string(placed.width);
}

void SceneGallery::sync() {
    auto& canvas = world_.get<engine::ui::UiCanvas>(canvas_);
    apply_overlay_canvas(canvas, pop_);
    vm_->galleryDisplay = pop_.is_open() ? "block" : "none";
    vm_->windowPop = pop_.scale();
    vm_->backdropDim = pop_.dim();
}

}
