#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace game {

// Empty dir reads as defaults and ignores writes.
class UserPrefs {
public:
    explicit UserPrefs(std::filesystem::path dir);

    // From %APPDATA%/electromagnetic-field when the new files are absent.
    void migrate_legacy();

    [[nodiscard]] std::string locale() const;
    void set_locale(std::string_view tag);

    [[nodiscard]] bool welcome_seen() const;
    void set_welcome_seen();

private:
    void write(const std::filesystem::path& path, std::string_view text) const;

    std::filesystem::path dir_;
};

}
