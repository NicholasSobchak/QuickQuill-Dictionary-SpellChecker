#include "SpellChecker.h"
#include "Utils.h"

SpellChecker::SpellChecker(const Dictionary &dict) : m_dict{ dict } {}

std::vector<std::string> SpellChecker::suggest(std::string_view prefix) const
{
    std::vector<std::string> results;
    if (prefix.empty()) return results;

    auto clean = dct::sanitizeWord(prefix);
    if (clean.empty()) return results;

    m_dict.suggestFromPrefix(clean, results, dct::g_maxSuggest);

    return results;
}

std::vector<std::string> SpellChecker::correct(std::string_view word) const
{
    std::vector<std::string> results;
    if (word.empty()) return results;

    auto clean = dct::sanitizeWord(word);
    if (clean.empty()) return results;

    if (m_dict.contains(clean))
    {
        results.push_back(clean);
        return results;
    }

    m_dict.suggestFromPrefix(clean, results, dct::g_maxSuggest);
    if (!results.empty()) return results;

    if (clean.size() > 1)
    {
        m_dict.suggestFromPrefix(clean.substr(0, 1), results, dct::g_maxSuggest);
    }
    return results;
}

std::string SpellChecker::autofill(std::string_view word) const
{
    auto suggestions = suggest(word);
    if (!suggestions.empty())
    {
        return suggestions.front();
    }
    return std::string(word);
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
			std::cout << "→ " << word << '\n'; 
		}
	}
}
