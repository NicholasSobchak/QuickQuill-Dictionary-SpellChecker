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
    friend class Tester; 

    Dictionary();
    ~Dictionary() = default;

	bool loadInfo(const std::string &filename); // populate database (file only)	
	WordInfo getWordInfo(std::string_view word) const;
	bool contains(std::string_view word) const;
	bool removeWord(const std::string& word);	
	
	void suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const;

private:	
	// Cache storage
	std::unordered_map<int, WordInfo> m_cache; // IMPLEMENT CACHE
	
	// Internals
	Trie m_trie;
	Database m_db;

    /*********************************
    // Helper declarations go here
    **********************************/	
	std::string cleanWord(std::string_view word) const; 
	void loadTrie(Database &db); // populate Trie using Database 
    bool loadjson(const std::string &filename); 
};
#endif
