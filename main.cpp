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
	void testDumpWord(std::string_view word) { dict_.m_trie.dumpWord(word); }
	void testGetWordInfo(std::string_view word) 
	{ 
		WordInfo info = dict_.getWordInfo(word);
		dict_.printInfo(info);
	}	


private:
	Dictionary& dict_;
	SpellChecker checker_;
};
int main()
{
	Dictionary dict;
	
	Tester test(dict);

	test.testDump();
	test.testGetWordInfo("dictionary");	

    return 0;
}

