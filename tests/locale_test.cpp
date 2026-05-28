#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {

std::unordered_map<std::string, std::string> load_table(const std::filesystem::path& path) {
    std::ifstream in(path);
    std::unordered_map<std::string, std::string> rows;
    std::string line;
    std::string id;
    while (std::getline(in, line)) {
        constexpr std::string_view kId = "id = \"";
        constexpr std::string_view kText = "text = \"";
        if (line.starts_with(kId) && line.ends_with('"')) {
            id = line.substr(kId.size(), line.size() - kId.size() - 1);
        } else if (!id.empty() && line.starts_with(kText) && line.ends_with('"')) {
            rows.emplace(std::move(id), line.substr(kText.size(), line.size() - kText.size() - 1));
            id.clear();
        }
    }
    return rows;
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream in(path);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

}

TEST(Locale, EnglishIsTheSourceAndMatchesUkrainianKeys) {
    const std::filesystem::path root{GAME_SOURCE_DIR};
    const auto english = load_table(root / "assets/locale/en.strings");
    const auto ukrainian = load_table(root / "assets/locale/uk.strings");
    ASSERT_GT(english.size(), 400u);
    ASSERT_EQ(english.size(), ukrainian.size());
    for (const auto& [id, text] : english) {
        ASSERT_FALSE(text.empty()) << id;
        const auto translated = ukrainian.find(id);
        ASSERT_NE(translated, ukrainian.end()) << id;
        EXPECT_FALSE(translated->second.empty()) << id;
    }

    for (const char* key : {"status.scene", "status.paused", "status.running", "status.scene_empty", "panel.pause",
                 "panel.resume", "help.reveal.show", "help.reveal.hide", "preset.dipole", "preset.rutherford",
                 "help.section.basics", "help.using.title", "panel.title", "welcome.start", "welcome.note"}) {
        EXPECT_TRUE(english.contains(key)) << key;
    }
    EXPECT_NE(english.at("status.scene").find("{name}"), std::string::npos);

    const std::string english_meta = read_file(root / "assets/locale/en.strings.meta");
    const std::string ukrainian_meta = read_file(root / "assets/locale/uk.strings.meta");
    EXPECT_NE(english_meta.find("source = true"), std::string::npos);
    EXPECT_EQ(ukrainian_meta.find("source = true"), std::string::npos);
    EXPECT_NE(read_file(root / "assets/locale/en.strings").find("locale = \"en\""), std::string::npos);
    EXPECT_NE(read_file(root / "assets/locale/uk.strings").find("locale = \"uk\""), std::string::npos);
}
