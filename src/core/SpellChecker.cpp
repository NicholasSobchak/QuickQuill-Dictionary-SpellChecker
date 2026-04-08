#include "SpellChecker.h"
#include "Utils.h"
#include <algorithm>
#include <unordered_set>

SpellChecker::SpellChecker(const Dictionary &dict) : m_dict{ dict } {}

std::vector<std::string> SpellChecker::suggest(std::string_view prefix) const
{
	std::vector<std::pair<std::string, dct::Frequency>> suggestions;
	if (prefix.empty()) return {};

	std::string clean = dct::sanitizeWord(prefix);
	if (clean.empty()) return {};

	// grabs words with the same prefix
    m_dict.suggestFromPrefix(clean, suggestions, dct::g_max_suggestions);

	std::vector<std::string> results;
	results.reserve(suggestions.size());
	for (const auto& p : suggestions) {
		results.push_back(p.first);
	}

    return results;
}

std::string SpellChecker::correct(std::string_view word) const
{
	std::string clean{ dct::sanitizeWord(word) };
	if (clean.empty()) return {};

	if (m_dict.contains(clean)) {
		return {};
	}

	std::vector<std::string> suggestions = m_dict.suggestSpelling(clean);

	if (!suggestions.empty()) {
		return suggestions[0]; // Return the first suggestion
	}

	return clean; // Return the original word if no suggestions are found
}

std::string SpellChecker::autofill(std::string_view word) const
{
	if (word.empty()) return {};

	std::vector<std::string> suggestions = suggest(word);

	if (!suggestions.empty()) {
		return suggestions[0]; // Return the first suggestion as the autocompletion
	}

	return std::string{ word }; // Return the original word if no suggestions found
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
