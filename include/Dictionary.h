#ifndef DICTIONARY_H
#define DICTIONARY_H
#include "WordInfo.h"
#include "Trie.h"
#include "Database.h"
#include "../nlohmann/json.hpp"
#include <unordered_map>
#include <fstream>

class Dictionary
{
public:
    explicit Dictionary(std::string dbPath = std::string(dct::g_dictDb));
    ~Dictionary() = default;

	WordInfo getWordInfo(std::string_view word) const;
	bool contains(std::string_view word) const;

	// bridge function from Trie to Spellchecker (placeholder)	
	void suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const;

private:	
	// Cache storage
	mutable std::unordered_map<int, WordInfo> m_cache; // mutable allows getWordInfo to be const
	
	Trie m_trie;
	Database m_db;

	std::string cleanWord(std::string_view word) const;
	void loadTrie(); // populate Trie using Database 

};
#endif
