#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <game/overlay_pop.h>
#include <game/scene.h>

#include <glm/vec2.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace game {

// Member names must match the binding paths in assets/ui/scenes.xml.
class SceneGalleryViewModel final : public engine::ui::ViewModel {
public:
    SceneGalleryViewModel();

    engine::ui::Bindable<std::string> galleryDisplay{std::string("none")};
    engine::ui::Bindable<std::string> windowPop{std::string("scale(0)")};
    engine::ui::Bindable<std::string> backdropDim{std::string("0")};
    engine::ui::Bindable<std::string> windowLeft{std::string("28")};
    engine::ui::Bindable<std::string> windowWidth{std::string("992")};
    engine::ui::RelayCommand closeGallery;

    engine::ui::RelayCommand presetDipole;
    engine::ui::RelayCommand presetLikePair;
    engine::ui::RelayCommand presetQuadrupole;
    engine::ui::RelayCommand presetCapacitor;
    engine::ui::RelayCommand presetOrbit;
    engine::ui::RelayCommand presetRutherford;
    engine::ui::RelayCommand presetSwarm;
    engine::ui::RelayCommand presetCyclotron;
    engine::ui::RelayCommand presetExBDrift;
    engine::ui::RelayCommand presetCoil;

    engine::ui::Bindable<bool> sceneDipole;
    engine::ui::Bindable<bool> sceneLikePair;
    engine::ui::Bindable<bool> sceneQuadrupole;
    engine::ui::Bindable<bool> sceneCapacitor;
    engine::ui::Bindable<bool> sceneOrbit;
    engine::ui::Bindable<bool> sceneRutherford;
    engine::ui::Bindable<bool> sceneSwarm;
    engine::ui::Bindable<bool> sceneCyclotron;
    engine::ui::Bindable<bool> sceneExBDrift;
    engine::ui::Bindable<bool> sceneCoil;
};

class SceneGallery {
public:
    using ChooseHandler = std::function<void(Preset)>;

    SceneGallery(engine::ecs::World& world, ChooseHandler on_choose);

    void open();
    void close();
    void tick(float dt);
    [[nodiscard]] bool is_open() const {
        return pop_.is_open();
    }

    void set_current(std::optional<Preset> preset);
    void update_layout(glm::ivec2 window_size);

private:
    void choose(Preset preset);
    void sync();

    engine::ecs::World& world_;
    engine::ecs::Entity canvas_{};
    std::shared_ptr<SceneGalleryViewModel> vm_;
    ChooseHandler on_choose_;
    OverlayPop pop_;
};

}
