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
// Nav row heights from assets/css/help.css (Button.nav-item 34 + list gap 2; header ~14 text + 16 margin + gap).
constexpr float kNavTopicRowHeight = 36.f;
constexpr float kNavHeaderRowHeight = 32.f;
// Rows kept visible beyond the selected one when the nav scrolls to it.
constexpr float kNavRevealContext = kNavTopicRowHeight;
// Vertical chrome around the nav list, mirroring assets/css/help.css: window top/bottom insets (28 + 28),
// nav padding (24 + 18) and the list's `calc(100% - 112px)`.
constexpr float kNavChromeHeight = 56.f + 42.f + 112.f;

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

constexpr const char* kBasics = "I. ОСНОВИ";
constexpr const char* kPotentialEnergy = "II. ПОТЕНЦІАЛ І ЕНЕРГІЯ";
constexpr const char* kSummary = "III. ПІДСУМОК";
constexpr const char* kAppendix = "ДОДАТОК";

// Order must match HelpViewModel::topics and the ItemsControls in assets/ui/help.xml.
// layers(potential, lines, grid, flow, probe, trails)
const std::array<HelpTopic, kHelpTopicCount> kTopics{
        HelpTopic{kBasics, "1. Електричний заряд", HelpDemo{Preset::Swarm, layers(true, true, false, true, true, true)}},
        HelpTopic{kBasics, "2. Закон Кулона", HelpDemo{Preset::Orbit, layers(true, true, false, false, true, true)}},
        HelpTopic{kBasics, "3. Електричне поле", HelpDemo{Preset::Dipole, layers(false, false, true, false, true, false)}},
        HelpTopic{kBasics, "4. Суперпозиція", HelpDemo{Preset::Quadrupole, layers(false, false, true, false, true, false)}},
        HelpTopic{kBasics, "5. Силові лінії", HelpDemo{Preset::Dipole, layers(false, true, false, true, true, false)}},
        HelpTopic{kBasics, "6. Потік і закон Гаусса", HelpDemo{Preset::Capacitor, layers(false, true, false, true, true, false)}},
        HelpTopic{kPotentialEnergy, "7. Потенціал", HelpDemo{Preset::Dipole, layers(true, false, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "8. Потенціал і енергія заряду", HelpDemo{Preset::LikePair, layers(true, false, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "9. Напруга й однорідне поле", HelpDemo{Preset::Capacitor, layers(true, true, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "10. Поле як нахил потенціалу", HelpDemo{Preset::Dipole, layers(true, false, true, false, true, false)}},
        HelpTopic{kPotentialEnergy, "11. Енергія пари зарядів", HelpDemo{Preset::Rutherford, layers(false, false, false, false, false, true)}},
        HelpTopic{kPotentialEnergy, "12. Нуль енергії і від'ємна U", HelpDemo{Preset::Orbit, layers(true, false, false, false, false, true)}},
        HelpTopic{kPotentialEnergy, "13. Робота поля і рух", HelpDemo{Preset::Orbit, layers(true, false, false, false, false, true)}},
        HelpTopic{kPotentialEnergy, "14. Енергія конфігурації", HelpDemo{Preset::Swarm, layers(true, false, false, false, false, true)}},
        HelpTopic{kSummary, "15. Електрика і гравітація", HelpDemo{Preset::Orbit, layers(false, true, false, false, false, true)}},
        HelpTopic{kSummary, "16. Загальна картина", HelpDemo{Preset::Dipole, layers(true, true, false, true, true, false)}},
        HelpTopic{kAppendix, "Як працює симуляція", HelpDemo{Preset::Swarm, layers(true, false, false, false, false, true)}},
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
    const char* section = nullptr;
    float row_y = 0.f;
    for (std::size_t i = 0; i < kTopics.size(); ++i) {
        if (section != kTopics[i].section) {
            section = kTopics[i].section;
            auto header = std::make_shared<HelpNavItem>();
            header->title = std::string(section);
            header->headerDisplay = std::string("block");
            header->topicDisplay = std::string("none");
            nav.push_back(std::move(header));
            row_y += kNavHeaderRowHeight;
        }
        nav_rows_[i] = nav.size();
        nav_row_y_[i] = row_y;
        row_y += kNavTopicRowHeight;
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
    nav_list_height_ = row_y;

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

void Help::select(std::size_t topic, bool reveal_in_nav) {
    if (topic >= kTopics.size()) {
        return;
    }
    if (topic != selected_) {
        selected_ = topic;
        vm_->contentScroll = 0.f;  // a new topic starts at its top
    }
    if (reveal_in_nav && nav_view_height_ > 0.f) {
        // Scroll only as far as needed to bring the row (plus a row of context) into view. The range is
        // clamped here: the engine clamps scroll offsets only when it re-lays-out, which a pure scroll
        // change does not trigger.
        const float top = nav_row_y_[topic];
        const float bottom = top + kNavTopicRowHeight;
        float scroll = vm_->navScroll.get();
        if (top - kNavRevealContext < scroll) {
            scroll = top - kNavRevealContext;
        } else if (bottom + kNavRevealContext > scroll + nav_view_height_) {
            scroll = bottom + kNavRevealContext - nav_view_height_;
        }
        const float max_scroll = std::max(0.f, nav_list_height_ - nav_view_height_);
        vm_->navScroll = std::clamp(scroll, 0.f, max_scroll);
    }
    sync();
}

void Help::select_previous() {
    select(selected_ == 0 ? kTopics.size() - 1 : selected_ - 1, true);
}

void Help::select_next() {
    select((selected_ + 1) % kTopics.size(), true);
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
    nav_view_height_ = std::max(0.f, static_cast<float>(window_size.y) - kNavChromeHeight);
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
        HelpNavItem& row = *nav[nav_rows_[i]];
        row.background = selected ? kNavSelectedBackground : kNavBackground;
        row.textColor = selected ? kNavSelectedText : kNavText;
    }
}

}
