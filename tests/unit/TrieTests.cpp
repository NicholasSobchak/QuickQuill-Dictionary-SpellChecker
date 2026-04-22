#include <catch2/catch_test_macros.hpp>

#include "Trie.h"

// insertion tests
TEST_CASE("reject duplicate insertions", "[trie]") {
  Trie trie;
  REQUIRE(trie.isEmpty());

  REQUIRE(trie.insert("had", dct::WordId{1}, dct::Frequency{1}));
  REQUIRE(trie.insert("hath", dct::WordId{2}, dct::Frequency{1}));
  REQUIRE_FALSE(trie.insert("hath", dct::WordId{3},
                            dct::Frequency{1})); // duplicate rejected

  REQUIRE(trie.contains("had"));
  REQUIRE(trie.contains("hath"));
  REQUIRE_FALSE(trie.contains("impaired"));
}

TEST_CASE("reject non-alpha input", "[trie]") {
  Trie trie;
  REQUIRE_FALSE(trie.insert("raven!", dct::WordId{1}, dct::Frequency{1}));
  REQUIRE_FALSE(trie.insert("tress01", dct::WordId{2}, dct::Frequency{1}));

  REQUIRE_FALSE(trie.contains("1she1"));
  REQUIRE_FALSE(trie.contains("%walks^!"));
}

TEST_CASE("case handling", "[trie]") {
  Trie trie;
  trie.insert("beauty", dct::WordId{1}, dct::Frequency{1});
  trie.insert("LIKE", dct::WordId{2}, dct::Frequency{1});

  REQUIRE(trie.contains("BEAUTY"));
  REQUIRE(trie.contains("like"));
}

TEST_CASE("insert long word and retrieve", "[trie]") {
  Trie trie;
  CHECK(trie.insert("hippopotomonstrosesquipedaliophobia", dct::WordId{1},
                    dct::Frequency{1})); // 35 characters

  REQUIRE(trie.contains("hippopotomonstrosesquipedaliophobia"));
  std::vector<std::pair<std::string, dct::Frequency>> out;
  trie.collectWithPrefix("hippo", out, 1);
  REQUIRE(out.size() == 1);
  CHECK(out[0].first == "hippopotomonstrosesquipedaliophobi"
                        "a"); // check collectWithPrefix
                              // return it
}

// id tests
TEST_CASE("getWordId returns correct id", "[trie]") {
  Trie trie;
  trie.insert("this", dct::WordId{42}, dct::Frequency{1});
  trie.insert("nameless", dct::WordId{7}, dct::Frequency{1});

  REQUIRE(trie.getWordId("this").value == 42);
  REQUIRE(trie.getWordId("nameless").value == 7);
  REQUIRE(trie.getWordId("name").value ==
          -1); // not the end of a word, default id is -1
}

TEST_CASE("getWordId returns correct id after removal", "[trie]") {
  Trie trie;
  trie.insert("thats", dct::WordId{1}, dct::Frequency{1});
  trie.insert("best", dct::WordId{2}, dct::Frequency{1});

  REQUIRE(trie.remove("thats"));
  REQUIRE(trie.getWordId("thats").value == -1);
  REQUIRE(trie.getWordId("best").value == 2);
}

// prefix tests
TEST_CASE("collectWithPrefix respects limit order", "[trie]") {
  Trie trie;
  // insert 3 words with the prefix "gra"
  trie.insert("grace", dct::WordId{1}, dct::Frequency{1});
  trie.insert("grandeur", dct::WordId{2}, dct::Frequency{1});
  trie.insert("gratitude", dct::WordId{3}, dct::Frequency{1});
  trie.insert("that", dct::WordId{4}, dct::Frequency{1});

  std::vector<std::pair<std::string, dct::Frequency>> out;
  trie.collectWithPrefix("gra", out, 2); // collect 2 words max
  REQUIRE(out.size() == 2);
  REQUIRE(out[0].first == "grace");
  REQUIRE(out[1].first == "grandeur");
}

TEST_CASE("prefix not present", "[trie]") {
  Trie trie;
  trie.insert("and", dct::WordId{1}, dct::Frequency{1});
  trie.insert("starry", dct::WordId{2}, dct::Frequency{1});
  trie.insert("skies", dct::WordId{3}, dct::Frequency{1});

  std::vector<std::pair<std::string, dct::Frequency>> out;
  trie.collectWithPrefix("all", out, 3);
  REQUIRE(out.size() == 0);
}

// removal tests
TEST_CASE("remove deletes only targeted word", "[trie]") {
  Trie trie;
  trie.insert("wave", dct::WordId{1}, dct::Frequency{1});
  trie.insert("waves", dct::WordId{2}, dct::Frequency{1});

  REQUIRE(trie.remove("wave"));
  REQUIRE_FALSE(trie.contains("wave"));
  REQUIRE(trie.contains("waves")); // sibling branch still intact
}

TEST_CASE("remove deletes only targeted word (single letter word)", "[trie]") {
  Trie trie;
  trie.insert("o", dct::WordId{1}, dct::Frequency{1});
  trie.insert("or", dct::WordId{2}, dct::Frequency{1});

  REQUIRE(trie.remove("o"));
  REQUIRE_FALSE(trie.contains("o"));
  REQUIRE(trie.contains("or"));
}

TEST_CASE("remove non-existent word", "[trie]") {
  Trie trie;
  trie.insert("the", dct::WordId{1}, dct::Frequency{1});
  trie.insert("night", dct::WordId{2}, dct::Frequency{1});

  REQUIRE_FALSE(trie.remove("of"));
  REQUIRE(trie.contains("the"));
  REQUIRE(trie.contains("night"));
}

TEST_CASE("removing only word leaves trie empty (only root)", "[trie]") {
  Trie trie;
  trie.insert("cloudless", dct::WordId{1}, dct::Frequency{1});

  REQUIRE(trie.remove("cloudless"));
  REQUIRE(trie.isEmpty());

  // prove trie is usuable (contains root)
  REQUIRE(trie.insert("climes", dct::WordId{2}, dct::Frequency{1}));
  REQUIRE(trie.contains("climes"));
}

// emtpy tests
TEST_CASE("clear() empties trie", "[trie]") {
  Trie trie;
  trie.insert("in", dct::WordId{1}, dct::Frequency{1});
  trie.clear();
  REQUIRE(trie.isEmpty());
  REQUIRE_FALSE(trie.contains("in"));
  REQUIRE(trie.getWordId("in").value == -1);
}

TEST_CASE("empty input", "[trie]") {
  Trie trie;
  REQUIRE_FALSE(trie.insert("", dct::WordId{1}, dct::Frequency{1}));

  REQUIRE_FALSE(trie.contains(""));
  REQUIRE(trie.getWordId("").value == -1);

  std::vector<std::pair<std::string, dct::Frequency>> out;
  trie.insert("ever", dct::WordId{1}, dct::Frequency{1});
  trie.insert("every", dct::WordId{2}, dct::Frequency{1});
  trie.collectWithPrefix("", out, 2); // collects nothing
  REQUIRE(out.size() == 0);
  trie.collectWithPrefix("eve", out,
                         0); // limit = 0 collects nothing (without traversal)
  REQUIRE(out.size() == 0);
}
