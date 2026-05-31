#include <game/welcome.h>

#include <game/locale_style.h>
#include <game/overlay_canvas.h>

#include <engine/loc/catalog.h>
#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <string>
#include <utility>

namespace game {
namespace {

constexpr int kWelcomeCanvasOrder = 30;  // above the reference (20) and the panel (10)
constexpr int kWindowMargin = 48;
constexpr int kMaxWindowWidth = 860;

}

WelcomeViewModel::WelcomeViewModel() {
    assets::ui::Welcome::bind(*this);
}

Welcome::Welcome(engine::ecs::World& world, LocaleHandler on_locale, DismissHandler on_dismiss) :
    world_(world),
    vm_(std::make_shared<WelcomeViewModel>()),
    on_locale_(std::move(on_locale)),
    on_dismiss_(std::move(on_dismiss)) {
    vm_->start = [this] { close(); };
    vm_->localeEn = [this] { on_locale_("en"); };
    vm_->localeUk = [this] { on_locale_("uk"); };
    apply_locale();

    canvas_ = world.create();
    world.emplace<engine::ui::UiCanvas>(canvas_, engine::ui::UiCanvas{
            .document = assets::ui::welcome,
            .stylesheet = assets::css::welcome,
            .data_context = vm_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = kWelcomeCanvasOrder,
    });
    sync();
}

void Welcome::apply_locale() {
    const bool english = world_.ctx<engine::loc::Catalog>().active() == "en";
    paint_locale_segment(
            LocaleSegment{vm_->localeEnBg, vm_->localeEnFg, vm_->localeUkBg, vm_->localeUkFg}, english);
}

void Welcome::open() {
    if (pop_.open()) {
        sync();
    }
}

void Welcome::close() {
    if (!pop_.close()) {
        return;
    }
    sync();
    if (on_dismiss_) {
        on_dismiss_();
    }
}

void Welcome::tick(float dt) {
    if (pop_.tick(dt)) {
        sync();
    }
}

void Welcome::update_layout(glm::ivec2 window_size) {
    const PlacedWindow placed = place_window(window_size, kWindowMargin, kMaxWindowWidth);
    vm_->windowLeft = std::to_string(placed.left);
    vm_->windowWidth = std::to_string(placed.width);
}

void Welcome::sync() {
    auto& canvas = world_.get<engine::ui::UiCanvas>(canvas_);
    apply_overlay_canvas(canvas, pop_);
    vm_->welcomeDisplay = pop_.is_open() ? "block" : "none";
    vm_->windowPop = pop_.scale();
    vm_->backdropDim = pop_.dim();
}

}
