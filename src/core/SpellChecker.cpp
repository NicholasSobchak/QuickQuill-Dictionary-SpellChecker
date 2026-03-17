#include "SpellChecker.h"
#include "Utils.h"
#include <algorithm>
#include <unordered_set>

SpellChecker::SpellChecker(const Dictionary &dict) : m_dict{ dict } {}

std::vector<std::string> SpellChecker::suggest(std::string_view prefix) const
{
    std::vector<std::string> results;
    if (prefix.empty()) return results;

	std::string clean = dct::sanitizeWord(prefix);
    if (clean.empty()) return results;

	// grabs words with the same prefix
    m_dict.suggestFromPrefix(clean, results, dct::g_maxSuggest);

    return results;
}

std::string SpellChecker::correct(std::string_view word) const
{
	std::string clean{ dct::sanitizeWord(word) };
	if (clean.empty()) return {};
	if (m_dict.contains(clean)) return {};
	std::string result{ "" };


	// use a simple SQL LIKE prefix search to surface a close lemma
	return result;
}

std::string SpellChecker::autofill(std::string_view word) const
{
	std::string result{ word }; 
	return result;
}

void SpellChecker::printSuggest(const std::vector<std::string> &out) const
{
	if (out.empty()) 
	{
		return;
	} 
	else
	{
		for (const auto &word : out)
		{
			std::cout << "  → " << word << '\n'; 
		}
	}
}
