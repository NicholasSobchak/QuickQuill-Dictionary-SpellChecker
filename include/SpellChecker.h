#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H
#include "Dictionary.h"
#include <string>

class SpellChecker
{
public:
	explicit SpellChecker(const Dictionary &dict);
	~SpellChecker() = default;
	
	void printSuggest(const std::vector<std::string> &out) const; // placeholder to print suggestions
	
	std::vector<std::string> suggest(std::string_view prefix) const;
	std::string correct(std::string_view word) const;
	
	std::string autofill(std::string_view word) const;
private:
	const Dictionary &m_dict;

    /*********************************
    // Helper declarations go here
    **********************************/

};
#endif
