#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <future>

#include "MockDB.h"

using namespace std::chrono_literals;

TEST_CASE("Dictionary handles concurrent synonym lookups", "[dictionary][concurrency]")
{
  auto tmp = test_support::tempDbPath("qq_dictionary_concurrency.sqlite");
  auto db = test_support::makeFreshDb(tmp);
  test_support::seedWord(db, "behind", "at the back of", {"after", "rear", "back"});

  auto dict = test_support::makeDictionaryOnDb(tmp);
  REQUIRE(dict.contains("behind"));

  auto worker = [&dict]()
  {
    for (int i = 0; i < 200; ++i)
    {
      (void)dict.getWordInfo("behind");
      (void)dict.suggestSynonyms("behind");
      (void)dict.getAlternativeSearches("behind");
    }
  };

  auto a = std::async(std::launch::async, worker);
  auto b = std::async(std::launch::async, worker);

  REQUIRE(a.wait_for(2s) == std::future_status::ready);
  REQUIRE(b.wait_for(2s) == std::future_status::ready);
  a.get();
  b.get();
}
