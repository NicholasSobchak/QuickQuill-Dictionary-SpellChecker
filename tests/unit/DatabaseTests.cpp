#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <string>

#include "core/Trie.h"
#include "data/Database.h"
#include "dct/dct.h"

namespace
{

std::filesystem::path makeTempDbPath()
{
  auto dir = std::filesystem::temp_directory_path();
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  return dir / ("quickquill-test-" + std::to_string(stamp) + ".db");
}

struct TestDb
{
  std::filesystem::path path;
  Database db;

  TestDb() : path(makeTempDbPath()), db(path.string()) { db.createTables(); }

  ~TestDb()
  {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};
} // end namespace

TEST_CASE("createTables", "[database]")
{
  SECTION("when database is empty")
  {
    TestDb test;
    REQUIRE(test.db.isEmpty());
  }

  SECTION("when tables already exist")
  {
    TestDb test;
    test.db.createTables();
    REQUIRE(test.db.isEmpty());
  }
}

TEST_CASE("insertWord", "[database]")
{
  SECTION("new word returns valid id")
  {
    TestDb test;
    const auto hathId = test.db.insertWord("hath", "Hath", dct::Frequency{1});
    REQUIRE(hathId.value == 1);
  }

  SECTION("duplicate lemma returns same id")
  {
    TestDb test;
    const auto wid = test.db.insertWord("hath", "Hath", dct::Frequency{1});
    const auto dupeWid = test.db.insertWord("hath", "Hath", dct::Frequency{1});
    REQUIRE(wid.value == dupeWid.value);
  }

  SECTION("empty lemma returns g_defaultId")
  {
    TestDb test;
    const auto emptyWid = test.db.insertWord("", "", dct::Frequency{0});
    REQUIRE(emptyWid.value == dct::g_defaultId);
  }
}

TEST_CASE("insertSense", "[database]")
{
  SECTION("with pos")
  {
    TestDb test;
    const auto wid = test.db.insertWord("impaired", "Impaired", dct::Frequency{1});
    const auto senseId = test.db.insertSense(wid, "adj", "Rendered less effective");
    REQUIRE(senseId.value != dct::g_defaultId);
  }

  SECTION("without pos")
  {
    TestDb test;
    const auto wid = test.db.insertWord("nameless", "Nameless", dct::Frequency{1});
    const auto senseId = test.db.insertSense(wid, "", "Not having a name");
    REQUIRE(senseId.value != dct::g_defaultId);
  }

  SECTION("empty definition returns g_defaultId")
  {
    TestDb test;
    const auto wid = test.db.insertWord("emptydef", "EmptyDef", dct::Frequency{1});
    const auto senseId = test.db.insertSense(wid, "n.", "");
    REQUIRE(senseId.value == dct::g_defaultId);
  }

  SECTION("invalid word_id returns g_defaultId")
  {
    TestDb test;
    const auto senseId = test.db.insertSense(dct::WordId{9999}, "n.", "ghost");
    REQUIRE(senseId.value == dct::g_defaultId);
  }
}

#if 0
TEST_CASE("insertForm", "[database]")
{
  SECTION("with tag")
  {
  }

  SECTION("without tag")
  {
  }

  SECTION("without form")
  {
  }

  SECTION("invalid word_id returns g_defaultId")
  {
  }
}
#endif

TEST_CASE("streamAllWordsAndForms prefers lemma over form collisions", "[database]")
{
  TestDb td;

  const auto axisId = td.db.insertWord("axis", "axis", dct::Frequency{1});
  const auto axesId = td.db.insertWord("axes", "axes", dct::Frequency{10});
  REQUIRE(axisId.value != dct::g_defaultId);
  REQUIRE(axesId.value != dct::g_defaultId);

  REQUIRE(td.db.insertForm(axisId, "axes", "plural"));

  Trie trie;
  td.db.streamAllWordsAndForms([&](dct::WordId id, std::string_view text, dct::Frequency frequency)
                               { trie.insert(dct::sanitizeWord(text), id, frequency); });

  REQUIRE(trie.getWordId("axes").value == axesId.value);
}
