#pragma once

#include <engine/ecs/entity.h>
#include <engine/ecs/world.h>
#include <engine/ui/bindable.h>
#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <glm/vec2.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace game {

// Data context of assets/ui/welcome.xml. Member names match the XML binding paths.
class WelcomeViewModel final : public engine::ui::ViewModel {
public:
    WelcomeViewModel();

    engine::ui::Bindable<std::string> welcomeDisplay{std::string("none")};
    engine::ui::Bindable<std::string> windowLeft{std::string("48")};
    engine::ui::Bindable<std::string> windowWidth{std::string("860")};
    engine::ui::RelayCommand start;
    engine::ui::RelayCommand localeEn;
    engine::ui::RelayCommand localeUk;
    engine::ui::Bindable<std::string> localeEnBg{std::string("#6366f133")};
    engine::ui::Bindable<std::string> localeEnFg{std::string("#ffffff")};
    engine::ui::Bindable<std::string> localeUkBg{std::string("#ffffff14")};
    engine::ui::Bindable<std::string> localeUkFg{std::string("#aab1c3")};
};

// First screen (assets/ui/welcome.xml): what the program is, and every control drawn as a keycap.
// Shown until dismissed once; the panel button opens it again.
class Welcome {
public:
    using LocaleHandler = std::function<void(std::string_view locale)>;
    using DismissHandler = std::function<void()>;

    Welcome(engine::ecs::World& world, LocaleHandler on_locale, DismissHandler on_dismiss);

    void apply_locale();

    void open();
    void close();
    [[nodiscard]] bool is_open() const {
        return open_;
    }

    // Centres the window and caps its width. Call every frame.
    void update_layout(glm::ivec2 window_size);

private:
    void sync();

    engine::ecs::World& world_;
    engine::ecs::Entity canvas_{};
    std::shared_ptr<WelcomeViewModel> vm_;
    LocaleHandler on_locale_;
    DismissHandler on_dismiss_;
    bool open_ = false;
};

}
