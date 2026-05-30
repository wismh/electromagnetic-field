#include <game/panel_hud.h>

#include <game/game.h>
#include <game/help.h>

#include <engine/ui/canvas.h>

#include <asset_ids.h>

namespace game {

PanelHud::PanelHud(engine::ecs::World& world, Game& game, SimClock& clock, Help& help, SceneGallery& scenes) :
    world_(world),
    game_(game),
    clock_(clock),
    help_(help),
    scenes_(scenes),
    vm_(std::make_shared<PanelViewModel>()) {
    bind_commands();

    const engine::ecs::Entity canvas = world_.create();
    world_.emplace<engine::ui::UiCanvas>(canvas, engine::ui::UiCanvas{
            .document = assets::ui::panel,
            .stylesheet = assets::css::panel,
            .data_context = vm_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = 10,
    });
}

PanelViewModel& PanelHud::view_model() {
    return *vm_;
}

void PanelHud::toggle_pause() {
    clock_.paused = !clock_.paused;
}

void PanelHud::step() {
    clock_.paused = true;
    clock_.step_requested = true;
}

void PanelHud::clear() {
    game_.clear_charges();
}

void PanelHud::open_scenes() {
    if (help_.is_open()) {
        help_.close();
    }
    scenes_.open();
}

void PanelHud::open_help() {
    help_.open();
}

void PanelHud::locale_en() {
    game_.set_locale("en");
}

void PanelHud::locale_uk() {
    game_.set_locale("uk");
}

void PanelHud::bind_commands() {
    vm_->togglePause.bind_to<PanelHud, &PanelHud::toggle_pause>(*this);
    vm_->step.bind_to<PanelHud, &PanelHud::step>(*this);
    vm_->clearAll.bind_to<PanelHud, &PanelHud::clear>(*this);
    vm_->openScenes.bind_to<PanelHud, &PanelHud::open_scenes>(*this);
    vm_->openHelp.bind_to<PanelHud, &PanelHud::open_help>(*this);
    vm_->localeEn.bind_to<PanelHud, &PanelHud::locale_en>(*this);
    vm_->localeUk.bind_to<PanelHud, &PanelHud::locale_uk>(*this);
}

}
