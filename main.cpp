// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

class Tester {
	public: 
		Tester() : dict_(), checker_(dict_)
		{
			if (dict_.m_db.isEmpty()) 
			{
				dict_.loadInfo("nlohmann/testdb.json");
			}
		}

		// Dictionary
		//bool dictTestLoadInfo() 
		//{
		//	_dict.getWordInfo(testWord_); // adds word to cache 
		//	return _dict.contains(testWord_); 
		//}
		
		bool dictTestRemoveWord()
		{
			dict_.removeWord(testWord_);
			return (!dict_.m_db.contains(testWord_) && dict_.m_trie.contains(testWord_) && dict_.contains(testWord_));
		}


		// Trie
		bool trieTestContains() { return dict_.m_trie.contains(testWord_); }

		void trieTestDump() { dict_.m_trie.dump(); }

		int trieTestGetWordId() { return dict_.m_trie.getWordId(testWord_); }


		// Database
		bool databaseTestClear()
		{
			dict_.m_db.clearDB();
			return (!dict_.m_db.isEmpty());
		}

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

