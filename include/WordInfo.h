#ifndef WORDINFO_H
#define WORDINFO_H
#include <string>
#include <vector>

struct Sense
{
	int id{};
	std::string pos; // noun, verb, adj, etc.
	std::string definition;
	std::vector<std::string> examples;
    std::vector<std::string> synonyms;
    std::vector<std::string> antonyms;
};

// plurals or alternative spellings
struct Form
{
	std::string form;
	std::string tag; 
};

struct WordInfo
{
	int id{};   
    std::string lemma;
	std::string displayLemma;	

	std::vector<std::string> etymology;
	std::vector<Form> forms; 
    std::vector<Sense> senses; 
};

#endif
