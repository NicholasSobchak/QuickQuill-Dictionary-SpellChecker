// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

class Tester
{
public:
	Tester(Dictionary& d) 
		: dict_{ d }, checker_{ dict_ } {}

	void testDump() { dict_.m_trie.dump(); }
	void testDBdump(std::string_view word) { dict_.m_db.dumpWord(word); }
	void testDumpWord(std::string_view word) { dict_.m_trie.dumpWord(word); }
	


private:
	Dictionary& dict_;
	SpellChecker checker_;
};
int main()
{
	Dictionary dict;
	
	Tester test(dict);
	
	test.testDump();
	test.testDBdump("dictionary");
	test.testDBdump("bright");
	test.testDumpWord("running");

    return 0;
}

