#include <game/help.h>

#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace game {
namespace {

constexpr int kHelpCanvasOrder = 20;  // above the control panel (10)
constexpr int kWindowMargin = 36;
constexpr int kMaxWindowWidth = 1280;

constexpr const char* kNavSelectedBackground = "#6366f133";
constexpr const char* kNavBackground = "#00000000";
constexpr const char* kNavSelectedText = "#ffffff";
constexpr const char* kNavText = "#aab1c3";

FieldLayers layers(bool potential, bool lines, bool grid, bool flow, bool probe, bool trails) {
    return FieldLayers{
            .potential = potential,
            .lines = lines,
            .grid = grid,
            .flow = flow,
            .probe = probe,
            .trails = trails,
    };
}

// Order must match HelpViewModel::topics and the ItemsControls in assets/ui/help.xml.
const std::array<HelpTopic, kHelpTopicCount> kTopics{
        HelpTopic{"1. Електричний заряд", HelpDemo{Preset::Swarm, layers(true, true, false, true, true, true)}},
        HelpTopic{"2. Закон Кулона", HelpDemo{Preset::Orbit, layers(true, true, false, false, true, true)}},
        HelpTopic{"3. Електричне поле", HelpDemo{Preset::Dipole, layers(false, false, true, false, true, false)}},
        HelpTopic{"4. Суперпозиція", HelpDemo{Preset::Quadrupole, layers(false, false, true, false, true, false)}},
        HelpTopic{"5. Силові лінії", HelpDemo{Preset::Dipole, layers(false, true, false, true, true, false)}},
        HelpTopic{"6. Потенціал", HelpDemo{Preset::LikePair, layers(true, false, false, false, true, false)}},
        HelpTopic{"7. Енергія", HelpDemo{Preset::Orbit, layers(true, false, false, false, false, true)}},
        HelpTopic{"8. Рух і кроки в часі", HelpDemo{Preset::Orbit, layers(false, false, false, false, false, true)}},
        HelpTopic{"9. Згладжування", HelpDemo{Preset::Swarm, layers(true, false, false, false, true, true)}},
        HelpTopic{"10. Зіткнення", HelpDemo{Preset::Swarm, layers(true, false, false, false, false, true)}},
        HelpTopic{"11. Сцени як досліди", HelpDemo{Preset::Rutherford, layers(false, false, false, false, false, true)}},
        HelpTopic{"12. Одиниці", std::nullopt},
        HelpTopic{"13. Межі моделі", std::nullopt},
};

}

std::span<const HelpTopic, kHelpTopicCount> help_topics() {
    return kTopics;
}

Help::Help(engine::ecs::World& world, DemoHandler on_demo) :
    world_(world),
    vm_(std::make_shared<HelpViewModel>()),
    on_demo_(std::move(on_demo)) {
    vm_->closeHelp = [this] { close(); };

    std::vector<std::shared_ptr<HelpNavItem>> nav;
    for (std::size_t i = 0; i < kTopics.size(); ++i) {
        auto item = std::make_shared<HelpNavItem>();
        item->title = std::string(kTopics[i].title);
        item->select = [this, i] { select(i); };
        nav.push_back(std::move(item));

        auto topic = std::make_shared<HelpTopicViewModel>();
        if (kTopics[i].demo) {
            topic->tryIt = [this, i] {
                close();
                on_demo_(*kTopics[i].demo);
            };
        }
        vm_->topics[i]->set({std::move(topic)});
    }
    vm_->navItems.set(std::move(nav));

    canvas_ = world.create();
    world.emplace<engine::ui::UiCanvas>(canvas_, engine::ui::UiCanvas{
            .document = assets::ui::help,
            .stylesheet = assets::css::help,
            .data_context = vm_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = kHelpCanvasOrder,
    });
    sync();
}

void Help::open() {
    open_ = true;
    sync();
}

void Help::close() {
    open_ = false;
    sync();
}

void Help::toggle() {
    open_ ? close() : open();
}

void Help::select(std::size_t topic) {
    if (topic >= kTopics.size()) {
        return;
    }
    if (topic != selected_) {
        selected_ = topic;
        vm_->contentScroll = 0.f;  // a new topic starts at its top
    }
    sync();
}

void Help::select_previous() {
    select(selected_ == 0 ? kTopics.size() - 1 : selected_ - 1);
}

void Help::select_next() {
    select((selected_ + 1) % kTopics.size());
}

void Help::try_selected() {
    if (kTopics[selected_].demo) {
        close();
        on_demo_(*kTopics[selected_].demo);
    }
}

// The cap is an explicit px width rather than CSS max-width: while measuring content height the engine
// resolves children's percentage widths against the parent's *unclamped* width (content_basis ignores
// max-width), so wrapped text under a max-width box is measured wider than it is laid out and the
// scroll range comes out short. See docs/engine-limits.md.
void Help::update_layout(glm::ivec2 window_size) {
    const int width = std::max(0, std::min(window_size.x - 2 * kWindowMargin, kMaxWindowWidth));
    const int left = (window_size.x - width) / 2;
    vm_->windowLeft = std::to_string(left);
    vm_->windowWidth = std::to_string(width);
}

void Help::sync() {
    // The engine routes the pointer to the highest-order canvas whose rect contains it, whether or
    // not anything there is clickable. A closed full-window help canvas would therefore swallow every
    // click meant for the panel below, so while closed it gets an empty Fixed rect instead.
    auto& canvas = world_.get<engine::ui::UiCanvas>(canvas_);
    canvas.fit = open_ ? engine::ui::UiFit::FillWindow : engine::ui::UiFit::Fixed;
    if (!open_) {
        canvas.rect = {};
    }
    vm_->helpDisplay = open_ ? "block" : "none";
    const auto& nav = vm_->navItems.get();
    for (std::size_t i = 0; i < kTopics.size(); ++i) {
        const bool selected = i == selected_;
        *vm_->topic_displays[i] = selected ? "block" : "none";
        nav[i]->background = selected ? kNavSelectedBackground : kNavBackground;
        nav[i]->textColor = selected ? kNavSelectedText : kNavText;
    }
}

}
