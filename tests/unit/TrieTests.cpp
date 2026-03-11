//AL
#define CATCH_CONFIG_MAIN
#include "third_party/catch2/catch.hpp"
#include "include/Trie.h"
#include <vector>
#include <string>

TEST_CASE("Trie insert and contains", "[trie]")
{
    Trie trie;
    REQUIRE(trie.insert("cat", 1));
    REQUIRE(trie.insert("car", 2));
    REQUIRE(trie.insert("dog", 3));

    CHECK(trie.contains("cat"));
    CHECK(trie.contains("car"));
    CHECK_FALSE(trie.contains("cot"));
}

TEST_CASE("Trie word ids", "[trie]")
{
    Trie trie;
    trie.insert("cat", 1);
    trie.insert("dog", 2);

    REQUIRE(trie.getWordId("cat") == 1);
    REQUIRE(trie.getWordId("dog") == 2);
    REQUIRE(trie.getWordId("bird") == dct::g_defaultId);
}

TEST_CASE("Trie collectWithPrefix respects limit", "[trie]")
{
    Trie trie;
    trie.insert("apple", 1);
    trie.insert("app", 2);
    trie.insert("application", 3);

    std::vector<std::string> results;
    trie.collectWithPrefix("app", results, /*limit*/ 2);
    REQUIRE(results.size() <= 2);
    for (const auto &word : results)
    {
        CHECK(word.rfind("app", 0) == 0); // prefix check
    }
}

TEST_CASE("Trie handles empty and clear", "[trie]")
{
    Trie trie;
    CHECK(trie.isEmpty());
    trie.insert("cat", 1);
    CHECK_FALSE(trie.isEmpty());
    trie.clear();
    CHECK(trie.isEmpty());
    CHECK_FALSE(trie.contains("cat"));
}

TEST_CASE("Trie remove deletes target but keeps siblings", "[trie]")
{
    Trie trie;
    trie.insert("car", 1);
    trie.insert("card", 2);
    trie.insert("cart", 3);
    trie.insert("dog", 4);

    REQUIRE(trie.remove("card"));
    CHECK_FALSE(trie.contains("card"));
    CHECK(trie.contains("car"));
    CHECK(trie.contains("cart"));
    CHECK(trie.contains("dog")); // different branch unaffected

    CHECK_FALSE(trie.remove("card")); // removing again should fail
}

TEST_CASE("Trie insert rejects invalid words", "[trie]")
{
    Trie trie;
    CHECK_FALSE(trie.insert("", 1));
    CHECK(trie.isEmpty());

    CHECK_FALSE(trie.insert("c@t", 2)); // non-alpha stops insert mid-way
    CHECK_FALSE(trie.contains("c@t"));
    CHECK_FALSE(trie.contains("ct"));
    CHECK(trie.getWordId("c@t") == dct::g_defaultId);
}

TEST_CASE("Trie getPrefix follows only existing nodes", "[trie]")
{
    Trie trie;
    trie.insert("hello", 1);
    trie.insert("helium", 2);

    CHECK(trie.getPrefix("help") == "hel"); // diverges at 'p'
    CHECK(trie.getPrefix("hex") == "he");
    CHECK(trie.getPrefix("h") == "h");
    CHECK(trie.getPrefix("zoo").empty());
}

TEST_CASE("Trie collectWithPrefix on missing prefix returns empty", "[trie]")
{
    Trie trie;
    trie.insert("alpha", 1);
    std::vector<std::string> out;
    trie.collectWithPrefix("beta", out, 5);
    CHECK(out.empty());
}

TEST_CASE("Trie collectWithPrefix honors zero limit", "[trie]")
{
    Trie trie;
    trie.insert("alpha", 1);
    std::vector<std::string> out;
    trie.collectWithPrefix("a", out, 0);
    CHECK(out.empty());
}

TEST_CASE("Trie rejects duplicate inserts and keeps original id", "[trie]")
{
    Trie trie;
    REQUIRE(trie.insert("cat", 1));
    CHECK_FALSE(trie.insert("cat", 2)); // duplicate
    CHECK(trie.getWordId("cat") == 1);
}

TEST_CASE("Trie remove single-letter word resets to empty", "[trie]")
{
    Trie trie;
    REQUIRE(trie.insert("a", 5));
    CHECK_FALSE(trie.isEmpty());
    REQUIRE(trie.remove("a"));
    CHECK(trie.isEmpty());
    CHECK_FALSE(trie.contains("a"));
}

TEST_CASE("Trie handles mixed case input", "[trie]")
{
    Trie trie;
    REQUIRE(trie.insert("Cat", 7)); // indexForChar lowercases
    CHECK(trie.contains("cat"));
    CHECK(trie.contains("Cat"));
    CHECK(trie.getWordId("CAT") == 7);
}

TEST_CASE("Trie collectWithPrefix returns all when limit big", "[trie]")
{
    Trie trie;
    trie.insert("bat", 1);
    trie.insert("batch", 2);
    trie.insert("battery", 3);

    std::vector<std::string> out;
    trie.collectWithPrefix("bat", out, 10);
    CHECK(out.size() == 3);
}

TEST_CASE("Trie contains and getWordId ignore non-alpha input", "[trie]")
{
    Trie trie;
    trie.insert("cat", 1);
    CHECK_FALSE(trie.contains("c@t"));
    CHECK(trie.getWordId("c@t") == dct::g_defaultId);
    CHECK(trie.contains("cat"));
}
