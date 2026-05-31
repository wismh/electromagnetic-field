#include <game/user_prefs.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <random>
#include <string>

namespace {

class TempDir {
public:
    TempDir() {
        path_ = std::filesystem::temp_directory_path() / "emf-user-prefs" /
                std::to_string(std::random_device{}());
        std::filesystem::create_directories(path_);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    [[nodiscard]] const std::filesystem::path& path() const {
        return path_;
    }

private:
    std::filesystem::path path_;
};

}

TEST(UserPrefs, MissingFilesReadAsDefaults) {
    const TempDir dir;
    const game::UserPrefs prefs(dir.path());
    EXPECT_EQ(prefs.locale(), "en");
    EXPECT_FALSE(prefs.welcome_seen());
}

TEST(UserPrefs, RoundTrip) {
    const TempDir dir;
    {
        game::UserPrefs prefs(dir.path());
        prefs.set_locale("uk");
        prefs.set_welcome_seen();
    }
    const game::UserPrefs again(dir.path());
    EXPECT_EQ(again.locale(), "uk");
    EXPECT_TRUE(again.welcome_seen());
}

TEST(UserPrefs, UnknownTagIsNotWritten) {
    const TempDir dir;
    game::UserPrefs prefs(dir.path());
    prefs.set_locale("uk");
    prefs.set_locale("fr");
    EXPECT_EQ(game::UserPrefs(dir.path()).locale(), "uk");
}

TEST(UserPrefs, EmptyPathIgnoresWrites) {
    game::UserPrefs prefs({});
    prefs.set_locale("uk");
    prefs.set_welcome_seen();
    EXPECT_EQ(prefs.locale(), "en");
    EXPECT_FALSE(prefs.welcome_seen());
}

