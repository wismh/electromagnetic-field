#include <game/user_prefs.h>

#include <cstdlib>
#include <fstream>
#include <utility>

namespace game {

UserPrefs::UserPrefs(std::filesystem::path dir) :
    dir_(std::move(dir)) {}

void UserPrefs::migrate_legacy() {
    if (dir_.empty()) {
        return;
    }
    const bool locale_missing = !std::filesystem::exists(dir_ / "locale.txt");
    const bool welcome_missing = !std::filesystem::exists(dir_ / "welcome.txt");
    if (!locale_missing && !welcome_missing) {
        return;
    }
    // Legacy %APPDATA% folder. getenv: MSVC deprecates the secure variant.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    const char* appdata = std::getenv("APPDATA");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (appdata == nullptr || appdata[0] == '\0') {
        return;
    }
    const std::filesystem::path legacy = std::filesystem::path(appdata) / "electromagnetic-field";
    const auto copy_if = [&](const char* name, bool missing) {
        if (!missing) {
            return;
        }
        std::error_code ec;
        std::filesystem::copy_file(legacy / name, dir_ / name, ec);
    };
    copy_if("locale.txt", locale_missing);
    copy_if("welcome.txt", welcome_missing);
}

std::string UserPrefs::locale() const {
    if (dir_.empty()) {
        return "en";
    }
    std::ifstream in(dir_ / "locale.txt");
    std::string tag;
    if (in >> tag && (tag == "en" || tag == "uk")) {
        return tag;
    }
    return "en";
}

void UserPrefs::set_locale(std::string_view tag) {
    if (dir_.empty() || (tag != "en" && tag != "uk")) {
        return;
    }
    write(dir_ / "locale.txt", tag);
}

bool UserPrefs::welcome_seen() const {
    if (dir_.empty()) {
        return false;
    }
    std::ifstream in(dir_ / "welcome.txt");
    std::string flag;
    return static_cast<bool>(in >> flag) && flag == "seen";
}

void UserPrefs::set_welcome_seen() {
    if (dir_.empty()) {
        return;
    }
    write(dir_ / "welcome.txt", "seen");
}

void UserPrefs::write(const std::filesystem::path& path, std::string_view text) const {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (out) {
        out << text << '\n';
    }
}

}
