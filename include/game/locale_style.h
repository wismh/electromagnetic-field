#pragma once

#include <engine/ui/bindable.h>

#include <string>

namespace game {

inline constexpr const char* kLocaleOnBg = "#6366f133";
inline constexpr const char* kLocaleOnFg = "#ffffff";
inline constexpr const char* kLocaleOffBg = "#ffffff14";
inline constexpr const char* kLocaleOffFg = "#aab1c3";

struct LocaleSegment {
    engine::ui::Bindable<std::string>& en_bg;
    engine::ui::Bindable<std::string>& en_fg;
    engine::ui::Bindable<std::string>& uk_bg;
    engine::ui::Bindable<std::string>& uk_fg;
};

inline void paint_locale_segment(LocaleSegment segment, bool english) {
    segment.en_bg = english ? kLocaleOnBg : kLocaleOffBg;
    segment.en_fg = english ? kLocaleOnFg : kLocaleOffFg;
    segment.uk_bg = english ? kLocaleOffBg : kLocaleOnBg;
    segment.uk_fg = english ? kLocaleOffFg : kLocaleOnFg;
}

}
