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

TEST_CASE("Database(filename)", "[database]")
{
  SECTION("invalid filename throws std::runtime_error") { SKIP("not implemented yet"); }

  SECTION("valid file opens without throw and handle() returns non-null")
  {
    SKIP("not implemented yet");
  }

  SECTION("busy timeout is set (open two connections)") { SKIP("not implemented yet"); }
}

TEST_CASE("createTables", "[database]")
{
  SECTION("fresh db has all tables and all indexes (verify with sqlite3_master)")
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

TEST_CASE("isEmpty", "[database]")
{
  SECTION("empty db returns true")
  {
    TestDb test;
    REQUIRE(test.db.isEmpty());
  }

  SECTION("after inserting a word returns false")
  {
    TestDb test;
    test.db.insertWord("which", "Which", dct::Frequency{1});
    REQUIRE_FALSE(test.db.isEmpty());
  }

  SECTION("after clearDB returns true")
  {
    TestDb test;
    test.db.insertWord("which", "Which", dct::Frequency{1});
    CHECK_FALSE(test.db.isEmpty());
    test.db.clearDB();
    REQUIRE(test.db.isEmpty());
  }
}

TEST_CASE("contains", "[database]")
{
  SECTION("existing word returns true")
  {
    TestDb test;
    test.db.insertWord("waves", "Waves", dct::Frequency{1});
    REQUIRE(test.db.contains("waves"));

    SECTION("after clearDB, previously existing word returns false")
    {
      test.db.clearDB();
      REQUIRE_FALSE(test.db.contains("waves"));
    }
  }

  SECTION("non-existing word returns false")
  {
    TestDb test;
    test.db.insertWord("waves", "Waves", dct::Frequency{1});
    REQUIRE_FALSE(test.db.contains("in"));
  }

  SECTION("empty db returns false")
  {
    TestDb test;
    REQUIRE_FALSE(test.db.contains("waves"));
  }
}

TEST_CASE("insertWord", "[database]")
{
  SECTION("new word returns valid id")
  {
    TestDb test;
    const auto wid = test.db.insertWord("hath", "Hath", dct::Frequency{1});
    REQUIRE(wid.value == 1);
  }

  SECTION("new words gets autoincrement id (1, then 2, etc.)")
  {
    TestDb test;
    const auto wid1 = test.db.insertWord("hath", "Hath", dct::Frequency{1});
    const auto wid2 = test.db.insertWord("impaired", "Impaired", dct::Frequency{1});
    REQUIRE(wid1.value == 1);
    REQUIRE(wid2.value == 2);
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

TEST_CASE("insertEtymology", "[database]")
{
  SECTION("single etymology line returns true") { SKIP("not implemented yet"); }

  SECTION("multiple etymology lines, getInfo returns them in order")
  {
    SKIP("not implemented yet");
  }

  SECTION("Invalid word_id returns false") { SKIP("not implemented yet"); }

  SECTION("empty vector returns true (no rows inserted)") { SKIP("not implemented yet"); }
}

TEST_CASE("insertForm", "[database]")
{
  SECTION("with tag returns true")
  {
    TestDb test;
    const auto wid = test.db.insertWord("grace", "Grace", dct::Frequency{1});
    const auto formSucc = test.db.insertForm(wid, "graces", "plural");
    REQUIRE(formSucc == true);
  }

  SECTION("without tag returns true") { SKIP("not implemented yet"); }

  SECTION("multiple forms for same word all returned by getInfo") { SKIP("not implemented yet"); }

  SECTION("without form returns false") { SKIP("not implemented yet"); }

  SECTION("invalid word_id returns false") { SKIP("not implemented yet"); }
}

TEST_CASE("insertExample", "[database]")
{
  SECTION("valid sense_id returns true") { SKIP("not implemented yet"); }

  SECTION("invalid sense_id returns false") { SKIP("not implemented yet"); }

  SECTION("multiple examples all returned by getInfo") { SKIP("not implemented yet"); }

  SECTION("empty example string returns false") { SKIP("not implemented yet"); }
}

TEST_CASE("insertSynonym", "[database]")
{
  SECTION("valid sense_id returns true") { SKIP("not implemented yet"); }

  SECTION("invalid sense_id returns false") { SKIP("not implemented yet"); }

  SECTION("multiple synonyms all returned by getInfo") { SKIP("not implemented yet"); }

  SECTION("empty synonym string returns false") { SKIP("not implemented yet"); }
}

TEST_CASE("insertAntonym", "[database]")
{
  SECTION("valid sense_id returns true") { SKIP("not implemented yet"); }

  SECTION("invalid sense_id returns false") { SKIP("not implemented yet"); }

  SECTION("multiple antonyms all returned by getInfo") { SKIP("not implemented yet"); }

  SECTION("empty antonym string") { SKIP("not implemented yet"); }
}

TEST_CASE("clearDB", "[database]")
{
  SECTION("makes isEmpty true after insert") { SKIP("not implemented yet"); }

  SECTION("can insert new words after clear and contains returns false for old words")
  {
    SKIP("not implemented yet");
  }

  SECTION("calling clearDB on already empty db is safe") { SKIP("not implemented yet"); }

  SECTION("foreign key cascade does not cause errors during clearDB")
  {
    SKIP("not implemented yet");
  }
}

TEST_CASE("getInfo", "[database]")
{
  SECTION("returns correct lemma, displayLemma, frequency") { SKIP("not implemented yet"); }

  SECTION("correct etymology vector") { SKIP("not implemented yet"); }

  SECTION("correct forms vector") { SKIP("not implemented yet"); }

  SECTION("correct senses with pos and definition") { SKIP("not implemented yet"); }

  SECTION("each sense has correct examples, synonyms, antonyms") { SKIP("not implemented yet"); }

  SECTION("invalid word_id returns empty WordInfo") { SKIP("not implemented yet"); }

  SECTION("word with 0 senses returns empty senses vector") { SKIP("not implemented yet"); }

  SECTION("full roundtrip with all fields populated") { SKIP("not implemented yet"); }
}

TEST_CASE("findMatchingWordIds", "[database]")
{
  SECTION("match by lemma returns correct id") { SKIP("not implemented yet"); }

  SECTION("match by form returns correct id") { SKIP("not implemented yet"); }

  SECTION("word that is both lemma and form returns both ids (distinct)")
  {
    SKIP("not implemented yet");
  }

  SECTION("empty string returns empty vector") { SKIP("not implemented yet"); }

  SECTION("no match returns empty vector") { SKIP("not implemented yet"); }

  SECTION("multiple forms matching same word return the same word_id")
  {
    SKIP("not implemented yet");
  }
}

TEST_CASE("handle", "[database]")
{
  SECTION("returns non-null pointer to a valid sqlite3 handle") { SKIP("not implemented yet"); }

  SECTION("handle can be used directly for sql execution") { SKIP("not implemented yet"); }
}

TEST_CASE("streamAllWordsAndForms", "[database]")
{
  SECTION("lemma preferred over form collision")
  {
    TestDb test;

    const auto axisId = test.db.insertWord("axis", "axis", dct::Frequency{1});
    const auto axesId = test.db.insertWord("axes", "axes", dct::Frequency{10});
    REQUIRE(axisId.value != dct::g_defaultId);
    REQUIRE(axesId.value != dct::g_defaultId);

    REQUIRE(test.db.insertForm(axisId, "axes", "plural"));

    Trie trie;
    test.db.streamAllWordsAndForms(
        [&](dct::WordId id, std::string_view text, dct::Frequency frequency)
        { trie.insert(dct::sanitizeWord(text), id, frequency); });

    REQUIRE(trie.getWordId("axes").value == axesId.value);
  }

  SECTION("empty db: processor never called") { SKIP("not implemented yet"); }

  SECTION("single lemma: processor called once with correct id, text, frequency")
  {
    SKIP("not implemented yet");
  }

  SECTION("lemma with a form: processor called for both") { SKIP("not implemented yet"); }

  SECTION("order: lemmas come before forms, higher frequency first")
  {
    SKIP("not implemented yet");
  }
}

TEST_CASE("foreign key integrity", "[database]")
{
  SECTION("inserting sense with non-existent word_id fails gracefully")
  {
    SKIP("not implemented yet");
  }

  SECTION("inserting example with non-existent sense_id fails gracefully")
  {
    SKIP("not implemented yet");
  }

  SECTION("inserting synonym with non-existent sense_id fails gracefully")
  {
    SKIP("not implemented yet");
  }

  SECTION("inserting antonym with non-existent sense_id fails gracefully")
  {
    SKIP("not implemented yet");
  }
}
