#include <catch2/catch_test_macros.hpp>

#include "Database.h"
#include "Trie.h"
#include "Utils.h"

#include <filesystem>
#include <chrono>
#include <string>

namespace {

std::filesystem::path makeTempDbPath()
{
    auto dir = std::filesystem::temp_directory_path();
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return dir / ("quickquill-test-" + std::to_string(stamp) + ".db");
}

} // namespace

TEST_CASE("streamAllWordsAndForms prefers lemma over form collisions", "[database]")
{
    const auto dbPath = makeTempDbPath();

    {
        Database db{dbPath.string()};
        db.createTables();

        const auto axisId = db.insertWord("axis", "axis", dct::Frequency{1});
        const auto axesId = db.insertWord("axes", "axes", dct::Frequency{10});
        REQUIRE(axisId.value != dct::g_defaultId);
        REQUIRE(axesId.value != dct::g_defaultId);

        REQUIRE(db.insertForm(axisId, "axes", "plural"));

        Trie trie;
        db.streamAllWordsAndForms([&](dct::WordId id, std::string_view text,
                                      dct::Frequency frequency)
        {
            trie.insert(dct::sanitizeWord(text), id, frequency);
        });

        REQUIRE(trie.getWordId("axes").value == axesId.value);
    }

    std::error_code ec;
    std::filesystem::remove(dbPath, ec);
}
