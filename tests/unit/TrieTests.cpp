#include <catch2/catch_test_macros.hpp>
#include "Trie.h"

// insertion tests
TEST_CASE("reject duplicate insertions", "[trie]")
{
	Trie trie;
	REQUIRE(trie.isEmpty());

	REQUIRE(trie.insert("had", 1));
	REQUIRE(trie.insert("hath", 2));
	REQUIRE_FALSE(trie.insert("hath", 3)); // duplicate rejected

	REQUIRE(trie.contains("had"));
	REQUIRE(trie.contains("hath"));
	REQUIRE_FALSE(trie.contains("impaired")); 
}

TEST_CASE("reject non-alpha input", "[trie]")
{
	Trie trie;
	REQUIRE_FALSE(trie.insert("raven!", 1));
	REQUIRE_FALSE(trie.insert("tress01", 2));

	REQUIRE_FALSE(trie.contains("1she1"));
	REQUIRE_FALSE(trie.contains("%walks^!"));
}

TEST_CASE("case handling", "[trie]")
{
	Trie trie;
	trie.insert("beauty", 1);
	trie.insert("LIKE", 2);

	REQUIRE(trie.contains("BEAUTY"));
	REQUIRE(trie.contains("like"));
}

TEST_CASE("insert long word and retrieve", "[trie]")
{
	Trie trie;
	CHECK(trie.insert("hippopotomonstrosesquipedaliophobia", 1)); // 35 characters
	
	REQUIRE(trie.contains("hippopotomonstrosesquipedaliophobia"));
	std::vector<std::string> out;
	trie.collectWithPrefix("hippo", out, 1); 
	REQUIRE(out.size() == 1);
	CHECK(out[0] == "hippopotomonstrosesquipedaliophobia"); // check collectWithPrefix return it 
}

// id tests
TEST_CASE("getWordId returns correct id", "[trie]")
{
	Trie trie;
	trie.insert("this", 42);
	trie.insert("nameless", 7);

	REQUIRE(trie.getWordId("this") == 42);
	REQUIRE(trie.getWordId("nameless") == 7);
	REQUIRE(trie.getWordId("name") == -1); // not the end of a word, default id is -1
}

TEST_CASE("getWordId returns correct id after removal", "[trie]")
{
	Trie trie;
	trie.insert("thats", 1);
	trie.insert("best", 2);

	REQUIRE(trie.remove("thats"));
	REQUIRE(trie.getWordId("thats") == -1); 
	REQUIRE(trie.getWordId("best") == 2);
}

// prefix tests
TEST_CASE("collectWithPrefix respects limit order", "[trie]")
{
	Trie trie;
	// insert 3 words with the prefix "gra"
	trie.insert("grace", 1);
	trie.insert("grandeur", 2);
	trie.insert("gratitude", 3);
	trie.insert("that", 4);

	std::vector<std::string> out;
	trie.collectWithPrefix("gra", out, 2); // collect 2 words max
	REQUIRE(out.size() == 2);
	REQUIRE(out[0] == "grace");
	REQUIRE(out[1] == "grandeur");	
}

TEST_CASE("prefix not present", "[trie]")
{
	Trie trie;
	trie.insert("and", 1);
	trie.insert("starry", 2);
	trie.insert("skies", 3);

	std::vector<std::string> out;
	trie.collectWithPrefix("all", out, 3); 
	REQUIRE(out.size() == 0);
}

// removal tests
TEST_CASE("remove deletes only targeted word", "[trie]")
{
	Trie trie;
	trie.insert("wave", 1);
	trie.insert("waves", 2);

	REQUIRE(trie.remove("wave"));
	REQUIRE_FALSE(trie.contains("wave"));
	REQUIRE(trie.contains("waves")); // sibling branch still intact
}

TEST_CASE("remove deletes only targeted word (single letter word)", "[trie]")
{
	Trie trie;
	trie.insert("o", 1);
	trie.insert("or", 2);

	REQUIRE(trie.remove("o"));
	REQUIRE_FALSE(trie.contains("o"));
	REQUIRE(trie.contains("or")); 
}

TEST_CASE("remove non-existent word", "[trie]")
{
	Trie trie;
	trie.insert("the", 1);
	trie.insert("night", 2);

	REQUIRE_FALSE(trie.remove("of"));
	REQUIRE(trie.contains("the"));
	REQUIRE(trie.contains("night")); 
}

TEST_CASE("removing only word leaves trie empty (only root)", "[trie]")
{
	Trie trie;
	trie.insert("cloudless", 1);

	REQUIRE(trie.remove("cloudless"));
	REQUIRE(trie.isEmpty());

	// prove trie is usuable (contains root)
	REQUIRE(trie.insert("climes", 2));
	REQUIRE(trie.contains("climes"));
}

// emtpy tests
TEST_CASE("clear() empties trie", "[trie]")
{
	Trie trie;
	trie.insert("in", 1);
	trie.clear();
	REQUIRE(trie.isEmpty());
	REQUIRE_FALSE(trie.contains("in"));
    REQUIRE(trie.getWordId("in") == -1);
}

TEST_CASE("empty input", "[trie]")
{
	Trie trie;
	REQUIRE_FALSE(trie.insert("", 1));
	
	REQUIRE_FALSE(trie.contains(""));
	REQUIRE(trie.getWordId("") == -1);

	std::vector<std::string> out;
	trie.insert("ever", 1);
	trie.insert("every", 2);
	trie.collectWithPrefix("", out, 2); // collects nothing 
	REQUIRE(out.size() == 0);
	trie.collectWithPrefix("eve", out, 0); // limit = 0 collects nothing (without traversal)
	REQUIRE(out.size() == 0);
}


