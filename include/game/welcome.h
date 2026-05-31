#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <glm/vec2.hpp>

#include <game/locale_style.h>
#include <game/overlay_canvas.h>
#include <game/overlay_pop.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace game {

// Member names must match the binding paths in assets/ui/welcome.xml.
class WelcomeViewModel final : public engine::ui::ViewModel {
public:
    WelcomeViewModel();

    engine::ui::Bindable<std::string> welcomeDisplay{std::string("none")};
    engine::ui::Bindable<std::string> windowPop{std::string("scale(0)")};
    engine::ui::Bindable<std::string> backdropDim{std::string("0")};
    engine::ui::Bindable<std::string> windowLeft{std::string("48")};
    engine::ui::Bindable<std::string> windowWidth{std::string("860")};
    engine::ui::RelayCommand start;
    engine::ui::RelayCommand localeEn;
    engine::ui::RelayCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string(kLocaleOnBg)};
    engine::ui::Bindable<std::string> localeEnFg{std::string(kLocaleOnFg)};
    engine::ui::Bindable<std::string> localeUkBg{std::string(kLocaleOffBg)};
    engine::ui::Bindable<std::string> localeUkFg{std::string(kLocaleOffFg)};
};

class Welcome {
public:
    using LocaleHandler = std::function<void(std::string_view locale)>;
    using DismissHandler = std::function<void()>;

    Welcome(engine::ecs::World& world, LocaleHandler on_locale, DismissHandler on_dismiss);

    void apply_locale();

    void open();
    void close();
    void tick(float dt);
    [[nodiscard]] bool is_open() const {
        return pop_.is_open();
    }

    void update_layout(glm::ivec2 window_size);

private:
    void sync();

    engine::ecs::World& world_;
    engine::ecs::Entity canvas_{};
    std::shared_ptr<WelcomeViewModel> vm_;
    LocaleHandler on_locale_;
    DismissHandler on_dismiss_;
    OverlayPop pop_;
};

}
