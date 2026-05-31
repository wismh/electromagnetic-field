#include <game/help.h>

#include <game/locale_style.h>
#include <game/overlay_canvas.h>

#include <engine/loc/catalog.h>
#include <engine/ui/canvas.h>
#include <engine/ui/document.h>

#include <asset_ids.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace game {
namespace {

constexpr int kHelpCanvasOrder = 20;  // above the panel (10)
constexpr int kWindowMargin = 36;
constexpr int kMaxWindowWidth = 1280;
// help.css: nav-item 34 + gap 2; header text 14 + margin 16 + gap.
constexpr float kNavTopicRowHeight = 36.f;
constexpr float kNavHeaderRowHeight = 32.f;
constexpr float kNavRevealContext = kNavTopicRowHeight;
// help.css chrome: window insets 56, nav padding 42, list offset 144.
constexpr float kNavChromeHeight = 56.f + 42.f + 144.f;

constexpr const char* kNavSelectedBackground = "#6366f133";
constexpr const char* kNavBackground = "#00000000";
constexpr const char* kNavSelectedText = "#ffffff";
constexpr const char* kNavText = "#aab1c3";

FieldLayers layers(bool potential, bool lines, bool grid, bool flow, bool probe, bool trails, bool magnetic = false) {
    return FieldLayers{
            .potential = potential,
            .lines = lines,
            .grid = grid,
            .flow = flow,
            .probe = probe,
            .trails = trails,
            .magnetic = magnetic,
    };
}

constexpr const char* kUsing = "help.section.using";
constexpr const char* kBasics = "help.section.basics";
constexpr const char* kPotentialEnergy = "help.section.potential";
constexpr const char* kEnergy = "help.section.energy";
constexpr const char* kSummary = "help.section.summary";
constexpr const char* kMagnetic = "help.section.magnetic";
constexpr const char* kAppendix = "help.section.appendix";

// Same order as HelpViewModel::topics. layers(potential, lines, grid, flow, probe, trails, magnetic)
const std::array<HelpTopic, kHelpTopicCount> kTopics{
        HelpTopic{kUsing, "help.using.title", std::nullopt},
        HelpTopic{kBasics, "help.charge.title", HelpDemo{Preset::Swarm, layers(true, true, false, true, true, true)}},
        HelpTopic{kBasics, "help.coulomb.title", HelpDemo{Preset::Orbit, layers(true, true, false, false, true, true)}},
        HelpTopic{kBasics, "help.field.title", HelpDemo{Preset::Dipole, layers(false, false, true, false, true, false)}},
        HelpTopic{kBasics, "help.superposition.title", HelpDemo{Preset::Quadrupole, layers(false, false, true, false, true, false)}},
        HelpTopic{kBasics, "help.field_lines.title", HelpDemo{Preset::Dipole, layers(false, true, false, true, true, false)}},
        HelpTopic{kBasics, "help.gauss.title", HelpDemo{Preset::Capacitor, layers(false, true, false, true, true, false)}},
        HelpTopic{kPotentialEnergy, "help.potential.title", HelpDemo{Preset::Dipole, layers(true, false, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "help.potential_energy.title", HelpDemo{Preset::LikePair, layers(true, false, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "help.voltage.title", HelpDemo{Preset::Capacitor, layers(true, true, false, false, true, false)}},
        HelpTopic{kPotentialEnergy, "help.gradient.title", HelpDemo{Preset::Dipole, layers(true, false, true, false, true, false)}},
        HelpTopic{kEnergy, "help.pair_energy.title", HelpDemo{Preset::Rutherford, layers(false, false, false, false, false, true)}},
        HelpTopic{kEnergy, "help.energy_zero.title", HelpDemo{Preset::Orbit, layers(true, false, false, false, false, true)}},
        HelpTopic{kEnergy, "help.work_motion.title", HelpDemo{Preset::Orbit, layers(true, false, false, false, false, true)}},
        HelpTopic{kEnergy, "help.config_energy.title", HelpDemo{Preset::Swarm, layers(true, false, false, false, false, true)}},
        HelpTopic{kMagnetic, "help.bfield.title", HelpDemo{Preset::Coil, layers(false, false, false, false, true, false, true)}},
        HelpTopic{kMagnetic, "help.lorentz.title", HelpDemo{Preset::Cyclotron, layers(false, false, false, false, true, true, true)}},
        HelpTopic{kMagnetic, "help.cyclotron.title", HelpDemo{Preset::Cyclotron, layers(false, false, false, false, true, true, true)}},
        HelpTopic{kMagnetic, "help.exb.title", HelpDemo{Preset::ExBDrift, layers(false, true, false, false, true, true, true)}},
        HelpTopic{kMagnetic, "help.coil.title", HelpDemo{Preset::Coil, layers(false, false, false, false, true, true, true)}},
        HelpTopic{kSummary, "help.gravity.title", HelpDemo{Preset::Orbit, layers(false, true, false, false, false, true)}},
        HelpTopic{kSummary, "help.overview.title", HelpDemo{Preset::Dipole, layers(true, true, false, true, true, false)}},
        HelpTopic{kAppendix, "help.simulation.title", HelpDemo{Preset::Swarm, layers(true, false, false, false, false, true)}},
};

bool has_class(const engine::ui::Element& element, std::string_view name) {
    return std::find(element.classes.begin(), element.classes.end(), name) != element.classes.end();
}

float answer_content_height(const engine::ui::Element& clip) {
    float height = 0.f;
    for (const engine::ui::Element& child : clip.children) {
        if (child.kind == engine::ui::ElementKind::ItemTemplate || child.display_none) {
            continue;
        }
        height += child.layout_rect.h;
    }
    return height;
}

HelpTopicViewModel* topic_from(HelpViewModel& vm, const void* owner) {
    if (owner == nullptr) {
        return nullptr;
    }
    for (auto* list : vm.topics) {
        for (const auto& item : list->get()) {
            if (item.get() == owner) {
                return item.get();
            }
        }
    }
    return nullptr;
}

// generated_owner is copied onto descendants; a new owner starts the next topic.
void sample_tree(engine::ui::Element& element, HelpViewModel& vm, const void*& owner, HelpTopicViewModel*& topic,
        int& index) {
    if (element.kind == engine::ui::ElementKind::ItemTemplate || element.display_none) {
        return;
    }
    if (element.generated_owner != owner) {
        owner = element.generated_owner;
        topic = topic_from(vm, owner);
        index = 0;
    }
    if (topic != nullptr && has_class(element, "predict-a")) {
        topic->note_answer_height(static_cast<std::size_t>(index), answer_content_height(element));
        ++index;
    }
    for (engine::ui::Element& child : element.children) {
        sample_tree(child, vm, owner, topic, index);
    }
    for (engine::ui::Element& child : element.generated_items) {
        sample_tree(child, vm, owner, topic, index);
    }
}

}

std::span<const HelpTopic, kHelpTopicCount> help_topics() {
    return kTopics;
}

Help::Help(engine::ecs::World& world, DemoHandler on_demo, LocaleHandler on_locale) :
    world_(world),
    vm_(std::make_shared<HelpViewModel>()),
    on_demo_(std::move(on_demo)),
    on_locale_(std::move(on_locale)) {
    vm_->closeHelp = [this] { close(); };
    vm_->localeEn = [this] { on_locale_("en"); };
    vm_->localeUk = [this] { on_locale_("uk"); };

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
    apply_locale();

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

void Help::apply_locale() {
    const engine::loc::Catalog& catalog = world_.ctx<engine::loc::Catalog>();
    const auto text_of = [&](const char* key) { return catalog.text(key).text; };
    const std::string show = text_of("help.reveal.show");
    const std::string hide = text_of("help.reveal.hide");
    const auto& nav = vm_->navItems.get();
    std::size_t row = 0;
    const char* section = nullptr;
    for (std::size_t i = 0; i < kTopics.size(); ++i) {
        if (section != kTopics[i].section) {
            section = kTopics[i].section;
            nav[row]->title = text_of(section);
            ++row;
        }
        nav[row]->title = text_of(kTopics[i].title);
        ++row;
        vm_->topics[i]->get().front()->set_reveal_labels(show, hide);
    }
    const bool english = catalog.active() == "en";
    paint_locale_segment(LocaleSegment{vm_->localeEnBg, vm_->localeEnFg, vm_->localeUkBg, vm_->localeUkFg}, english);
}

void Help::open() {
    if (pop_.open()) {
        sync();
    }
}

void Help::close() {
    if (pop_.close()) {
        sync();
    }
}

void Help::toggle() {
    if (pop_.is_open() && !pop_.closing()) {
        close();
    } else {
        open();
    }
}

void Help::tick(float dt) {
    if (pop_.tick(dt)) {
        sync();
    }
    sample_answers();
}

void Help::sample_answers() {
    engine::ui::UiInstance* instance = world_.try_get<engine::ui::UiInstance>(canvas_);
    if (instance == nullptr) {
        return;
    }
    const void* owner = nullptr;
    HelpTopicViewModel* topic = nullptr;
    int index = 0;
    sample_tree(instance->document.root, *vm_, owner, topic, index);
    for (auto* list : vm_->topics) {
        for (const auto& item : list->get()) {
            item->apply_answer_motion();
        }
    }
}

void Help::select(std::size_t topic, bool reveal_in_nav) {
    if (topic >= kTopics.size()) {
        return;
    }
    if (topic != selected_) {
        selected_ = topic;
        vm_->contentScroll = 0.f;
    }
    if (reveal_in_nav && nav_view_height_ > 0.f) {
        // Clamp here: the engine clamps scroll only on relayout.
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

void Help::update_layout(glm::ivec2 window_size) {
    const PlacedWindow placed = place_window(window_size, kWindowMargin, kMaxWindowWidth);
    vm_->windowLeft = std::to_string(placed.left);
    vm_->windowWidth = std::to_string(placed.width);
    nav_view_height_ = std::max(0.f, static_cast<float>(window_size.y) - kNavChromeHeight);
}

void Help::sync() {
    auto& canvas = world_.get<engine::ui::UiCanvas>(canvas_);
    apply_overlay_canvas(canvas, pop_);
    vm_->helpDisplay = pop_.is_open() ? "block" : "none";
    vm_->windowPop = pop_.scale();
    vm_->backdropDim = pop_.dim();
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
