#include <catch2/catch_test_macros.hpp>

#include "KaikkiImporter.h"

#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

namespace {

std::filesystem::path makeTempDbPath()
{
    auto dir = std::filesystem::temp_directory_path();
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return dir / ("quickquill-import-test-" + std::to_string(stamp) + ".db");
}

} // namespace

TEST_CASE("importKaikkiJsonl imports lemma, form, and senses", "[import]")
{
    const auto dbPath = makeTempDbPath();

    std::istringstream jsonl(R"jsonl(
{"lang_code":"en","word":"cat","freq":12,"etymology_text":"From Latin cattus.","forms":[{"form":"cats","tags":["plural"]}],"pos":"noun","senses":[{"glosses":["A domesticated feline."],"examples":[{"text":"The cat slept."}],"synonyms":[{"word":"feline"}],"antonyms":[{"word":"dog"}]}]}
)jsonl");

    {
        Database db{dbPath.string()};
        KaikkiImportOptions options;
        options.progressInterval = 0;
        options.batchSize = 1;
        options.setFastPragmas = false;
        options.clearDatabase = true;

        const auto stats = importKaikkiJsonl(db, jsonl, options);
        REQUIRE(stats.entries == 1);
        REQUIRE(stats.words == 1);
        REQUIRE(stats.forms == 1);
        REQUIRE(stats.senses == 1);
        REQUIRE(stats.examples == 1);
        REQUIRE(stats.synonyms == 1);
        REQUIRE(stats.antonyms == 1);
        REQUIRE(stats.etymologies == 1);

        REQUIRE(db.contains("cat"));
        const auto ids = db.findMatchingWordIds("cats");
        REQUIRE(ids.size() == 1);

        const auto info = db.getInfo(ids[0]);
        REQUIRE(info.lemma == "cat");
        REQUIRE_FALSE(info.senses.empty());
        REQUIRE(info.senses[0].definition == "A domesticated feline.");
        REQUIRE(info.senses[0].examples.size() == 1);
    }

    std::error_code ec;
    std::filesystem::remove(dbPath, ec);
}
