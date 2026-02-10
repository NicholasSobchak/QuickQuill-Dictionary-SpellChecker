// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

#include <iostream>

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
    SpellChecker checker(dict);
	Tester test(dict);

    std::cout << "Dictionary Spell Checker\n";
    std::cout << "Type command (lookup <word>, suggest <prefix>, correct <word>, autofill <prefix>, exit):\n";

    std::string command;
    while (std::cout << "> " && std::cin >> command)
    {
        if (command == "exit") break;

        if (command == "lookup")
        {
            std::string word;
            std::cin >> word;
            WordInfo info = dict.getWordInfo(word);
            if (info.lemma.empty())
            {
                std::cout << "Word not found" << '\n';
            }
            else
            {
                dict.printInfo(info);
            }
        }
        else if (command == "suggest")
        {
            std::string prefix;
            std::cin >> prefix;
            auto suggestions = checker.suggest(prefix);
            checker.printSuggest(suggestions);
        }
        else if (command == "correct")
        {
            std::string word;
            std::cin >> word;
            auto corrections = checker.correct(word);
            if (corrections.empty())
            {
                std::cout << "No corrections found" << '\n';
            }
            else
            {
                checker.printSuggest(corrections);
            }
        }
        else if (command == "autofill")
        {
            std::string prefix;
            std::cin >> prefix;
            std::cout << checker.autofill(prefix) << '\n';
        }
        else
        {
            std::cout << "Unknown command" << '\n';
        }
    }

    return 0;
}

