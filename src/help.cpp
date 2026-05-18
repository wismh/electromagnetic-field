#include <game/help.h>

#include <engine/ui/canvas.h>

#include <asset_ids.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

namespace game {
namespace {

constexpr int kHelpCanvasOrder = 20;  // above the control panel (10)

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
    sync();

    const engine::ecs::Entity canvas = world.create();
    world.emplace<engine::ui::UiCanvas>(canvas, engine::ui::UiCanvas{
            .document = assets::ui::help,
            .stylesheet = assets::css::help,
            .data_context = vm_,
            .fit = engine::ui::UiFit::FillWindow,
            .order = kHelpCanvasOrder,
    });
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

void Help::sync() {
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
