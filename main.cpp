// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

class Tester {
	public: 
		Tester() : _db(dct::g_dictDb), _dict(), _checker(_dict)
		{
		    _dict.loadInfo("nlohmann/testdb.json");
			_dict.m_trie.dump();
		}

		//bool dictTestLoadInfo() { return _dict.contains("run"); }

		bool trieTestContains() { return _dict.m_trie.contains("run"); }

	private:
		Database _db;	
		Dictionary _dict;
		SpellChecker _checker;
};

int main()
{
	Tester test;
#if 1
//	std::cout << "Testing dictTestLoadInfo()..."
//		<< (test.dictTestLoadInfo() ? "passed" : "failed")
//		<< "\n\n";

	std::cout << "Testing trieTestContains()..."
		<< (test.trieTestContains() ? "passed" : "failed")
		<< "\n\n";	
#endif

    return 0;
}

