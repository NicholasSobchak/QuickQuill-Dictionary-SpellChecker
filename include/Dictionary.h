#ifndef DICTIONARY_H
#define DICTIONARY_H
#include "WordInfo.h"
#include "Trie.h"
#include "Database.h"
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

class Dictionary
{
public:
    explicit Dictionary();
    ~Dictionary() = default;

	WordInfo getWordInfo(std::string_view word) const;
	bool contains(std::string_view word) const;

	// spellchecking functions
	std::vector<std::string> suggestFromPrefix(std::string_view word) const;
	std::vector<std::string> suggestSpelling(std::string_view word) const;

private:	
	// Cache storage
	mutable std::unordered_map<int, WordInfo> m_cache; // mutable allows getWordInfo to be const
	
	Trie m_trie;
	Database m_db;

	std::string cleanWord(std::string_view word) const;
	void loadTrie(); // populate Trie using Database 
	std::unordered_set<std::string> collectSuggestedWords(std::string_view word) const;

};
#endif
