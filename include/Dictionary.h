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
    Dictionary();
    ~Dictionary() = default;

	bool loadInfo(const std::string &filename); // populate database (file only)	
	WordInfo getWordInfo(std::string_view word) const;
	bool contains(std::string_view word) const;

	// bridge function from Trie to Spellchecker (placeholder)	
	void suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const;

private:	
	// Cache storage
	mutable std::unordered_map<int, WordInfo> m_cache; 
	
	Trie m_trie;
	Database m_db;

    /*********************************
    // Helper declarations go here
    **********************************/	
	std::string cleanWord(std::string_view word) const;
	void loadTrie(); // populate Trie using Database 
    bool loadjson(const std::string &filename); // implementation purposes (use python import script) 

};
#endif
