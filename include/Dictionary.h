#ifndef DICTIONARY_H
#define DICTIONARY_H
#include "Trie.h"
#include "Database.h"
#include "../nlohmann/json.hpp"
#include <fstream>

class Tester;
//class Trie;
//class Database;

class Dictionary
{
public:
    friend class Tester;

    Dictionary();
    ~Dictionary() = default;

	// Dictionary
    bool addWord(std::string_view word); // fix logic
    bool removeWord(std::string_view word); // implement databse logic
	bool loadInfo(const std::string &filename);
	void suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const;
	bool search(std::string_view word) const; 
	
	// Trie wrapper 
	bool isTrieEmpty() const;
	bool trieContainsWord(std::string_view word) const;
	void printTrie() const; 
    void dumpTrie() const;
	void dumpTrieWord(std::string_view word) const;
    void clearTrie();

	// Database wrapper 
	bool isDBEmpty() const;	
	void clearDB();

private:
	// represent word info from the db in memory (acts a 'cache')
	struct WordInfo
	{
       	std::string lemma; // word
	    std::vector<std::string> etymology;
	    int id{-1}; // ??
        
	    // plurals or alternative spellings
	    struct Form
        {
	    	std::string form;
	    	std::string tag;
	    };
	    std::vector<Form> forms; 
        
	    struct Sense
	 	{
	 	    std::string pos; // noun, verb, adj, etc.
	 	    std::string definition;
	 	    std::vector<std::string> examples;
      	    std::vector<std::string> synonyms;
     	    std::vector<std::string> antonyms;
     	};
	    std::vector<Sense> senses; 
	};

    Trie m_trie;
	Database m_db;

    /*********************************
    // Helper declarations go here
    **********************************/
	
	std::string normalize(std::string_view word) const; 
	void buildTrie(Database &db); 
    bool loadjson(const std::string &filename); 
};
#endif
