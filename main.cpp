// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

class Tester {
	public: 
		Tester() : dict_(), checker_(dict_)
		{
			dict_.loadInfo("nlohmann/testdb.json");
		}

		//bool dictTestLoadInfo() 
		//{
		//	_dict.getWordInfo(testWord_); 
		//	return _dict.contains(testWord_); 
		//}

		bool trieTestContains() { return dict_.m_trie.contains(testWord_); }

		void trieTestDump() { dict_.m_trie.dump(); }

		int trieTestGetWordId() { return dict_.m_trie.getWordId(testWord_); }

	private:
		Dictionary dict_;
		SpellChecker checker_;

		const std::string testWord_{ "run" };
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

	std::cout << "Testing trieTestDump()...\n";
	test.trieTestDump();
	std::cout << "\n\n";

	std::cout << "Testing trieTestGetWordId()...\nID: "
		<< test.trieTestGetWordId()
		<< "\n\n";

#endif

    return 0;
}

