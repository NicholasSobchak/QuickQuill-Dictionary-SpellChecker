#include "http/services/WordService.h"

#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace http
{
namespace
{
	Dictionary& dict()
	{
		static Dictionary instance;
		return instance;
	}

	SpellChecker& checker()
	{
		static SpellChecker instance{ dict() };
		return instance;
	}
} // namespace

WordService::WordService(Dictionary& dict, SpellChecker& checker) : m_dict{ dict }, m_checker{ checker } {}

/**
 * Initialize dict(), checker(), and call warmupDictionary() once after wordService() is called
 */
WordService& wordService()
{
	static WordService instance{ dict(), checker() };
	[[maybe_unused]] static const bool warmed = [] { // lambda logic (const, the value should never change)
		instance.warmupDictionary(); 
		return true;
	}();

	/**
	 * alternatively, one could just call warmupDicitonary() like this
	 * static bool warm = false;
	 * if (!warm)
	 * instance.warmpuDictionary();
	 * warm = true;
	 */
	
	return instance;
}

/**
 * Removes random characters that could potentially corrupt the search query
 */
std::string WordService::decodeInput(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for (size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 2 >= in.size()) return "";
			auto hex = in.substr(i + 1, 2);
			char* end = nullptr;
			long val = std::strtol(hex.c_str(), &end, 16);
			if (end != hex.c_str() + 2) return "";
			out.push_back(static_cast<char>(val));
			i += 2;
		}
		else if (in[i] == '+')
		{
			out.push_back(' ');
		}
		else
		{
			out.push_back(in[i]);
		}
	}
	return out;
}

void WordService::warmupDictionary() const
{
	m_dict.getWordInfo("warmup");
}

/**
 * The main query funciton for the search bar
 */
SearchResult WordService::search(const std::string& word) const
{
	const std::string decoded = decodeInput(word);
	if (decoded.empty())
	{
		nlohmann::json body = {
			{"error", "Enter a valid word"}
		};
		return { body.dump(), 400 };
	}

	const bool allowedChars = std::all_of(decoded.begin(), decoded.end(), [](unsigned char c) {
		return std::isalpha(c) || c == '\'' || c == '-' || c == ' ' || c == '.';
	});

	const std::string sanitized = dct::sanitizeWord(decoded);
	if (!allowedChars || sanitized.empty())
	{
		nlohmann::json body = {
			{"error", "Enter a valid word"}
		};
		return { body.dump(), 400 };
	}

	WordInfo info = m_dict.getWordInfo(sanitized);
	if (info.lemma.empty())
	{
		const std::string correctWord = m_checker.correct(sanitized);
		nlohmann::json body = {
			{"query", sanitized},
			{"found", false},
			{"suggestion", correctWord}
		};
		return { body.dump(), 404 };
	}

	const auto alternativeSearches = m_dict.getAlternativeSearches(sanitized, info.id);
	return { toWordJson(info, decoded, alternativeSearches), 200 };
}

/**
 * Provides similar searches using the suggest function
 */
SuggestResult WordService::suggest(const std::string& word) const
{
	const std::string decoded = decodeInput(word);
	if (decoded.empty())
	{
		return { "[]", 200 };
	}

	const std::string sanitized = dct::sanitizeWord(decoded);
	if (sanitized.empty())
	{
		return { "[]", 200 };
	}

	std::vector<std::string> suggestions = m_checker.suggest(sanitized);
	nlohmann::json body = suggestions;
	return { body.dump(), 200 };
}
} // namespace http
