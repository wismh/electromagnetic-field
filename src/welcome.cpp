#include <game/welcome.h>

#include <engine/loc/catalog.h>
#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <algorithm>
#include <string>
#include <utility>

namespace game {
namespace {

constexpr int kWelcomeCanvasOrder = 30;  // above the reference (20) and the panel (10)
constexpr int kWindowMargin = 48;
constexpr int kMaxWindowWidth = 860;

constexpr const char* kLocaleOnBg = "#6366f133";
constexpr const char* kLocaleOnFg = "#ffffff";
constexpr const char* kLocaleOffBg = "#ffffff14";
constexpr const char* kLocaleOffFg = "#aab1c3";

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
    vm_->localeEnBg = english ? kLocaleOnBg : kLocaleOffBg;
    vm_->localeEnFg = english ? kLocaleOnFg : kLocaleOffFg;
    vm_->localeUkBg = english ? kLocaleOffBg : kLocaleOnBg;
    vm_->localeUkFg = english ? kLocaleOffFg : kLocaleOnFg;
}

void Welcome::open() {
    open_ = true;
    sync();
}

void Welcome::close() {
    if (!open_) {
        return;
    }
    open_ = false;
    sync();
    if (on_dismiss_) {
        on_dismiss_();
    }
}

void Welcome::update_layout(glm::ivec2 window_size) {
    // Explicit px width, not CSS max-width: see Help::update_layout and docs/engine-limits.md.
    const int width = std::max(0, std::min(window_size.x - 2 * kWindowMargin, kMaxWindowWidth));
    const int left = (window_size.x - width) / 2;
    vm_->windowLeft = std::to_string(left);
    vm_->windowWidth = std::to_string(width);
}

void Welcome::sync() {
    // A closed full-window canvas would swallow every click meant for the panel below it. While
    // closed it gets an empty Fixed rect instead. See docs/engine-limits.md.
    auto& canvas = world_.get<engine::ui::UiCanvas>(canvas_);
    canvas.fit = open_ ? engine::ui::UiFit::FillWindow : engine::ui::UiFit::Fixed;
    if (!open_) {
        canvas.rect = {};
    }
    vm_->welcomeDisplay = open_ ? "block" : "none";
}

}
