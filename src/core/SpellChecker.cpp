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

	// "bridge function" (spellchecker can't see Trie)
    m_dict.suggestFromPrefix(clean, results, dct::g_maxSuggest);

    return results;
}

std::string SpellChecker::correct(std::string_view word) const
{
    const std::string clean = dct::sanitizeWord(word);
    if (clean.empty()) return {};

    if (m_dict.contains(clean)) return clean;
		
	// CURRENT AI SOLUTION

    auto generateEdits = [](std::string_view base, std::unordered_set<std::string> &out) {
        static constexpr char letters[] = "abcdefghijklmnopqrstuvwxyz";

        // deletions
        for (std::size_t i = 0; i < base.size(); ++i)
        {
            std::string candidate(base);
            candidate.erase(i, 1);
            if (!candidate.empty()) out.insert(std::move(candidate));
        }

        // transpositions
        for (std::size_t i = 0; i + 1 < base.size(); ++i)
        {
            std::string candidate(base);
            std::swap(candidate[i], candidate[i + 1]);
            out.insert(std::move(candidate));
        }

        // substitutions
        for (std::size_t i = 0; i < base.size(); ++i)
        {
            std::string candidate(base);
            for (char c : letters)
            {
                if (candidate[i] == c) continue;
                candidate[i] = c;
                out.insert(candidate);
            }
        }

        // insertions
        for (std::size_t i = 0; i <= base.size(); ++i)
        {
            for (char c : letters)
            {
                std::string candidate(base);
                candidate.insert(i, 1, c);
                out.insert(std::move(candidate));
            }
        }
    };

    auto scoreCandidate = [&](const std::string &candidate) {
        const WordInfo info = m_dict.getWordInfo(candidate);
        const int richness = static_cast<int>(info.senses.size() + info.forms.size());

        std::size_t sharedPrefix = 0;
        const std::size_t n = std::min(clean.size(), candidate.size());
        while (sharedPrefix < n && clean[sharedPrefix] == candidate[sharedPrefix]) ++sharedPrefix;

        return std::pair<int, std::size_t>{ richness, sharedPrefix };
    };

    std::unordered_set<std::string> edits;
    edits.reserve(4096);
    generateEdits(clean, edits);

    std::string best;
    int bestRichness = -1;
    std::size_t bestPrefix = 0;

    for (const auto &candidate : edits)
    {
        if (!m_dict.contains(candidate)) continue;

        const auto [richness, sharedPrefix] = scoreCandidate(candidate);
        const bool better = (richness > bestRichness) ||
                            (richness == bestRichness && sharedPrefix > bestPrefix) ||
                            (richness == bestRichness && sharedPrefix == bestPrefix && (best.empty() || candidate < best));
        if (better)
        {
            best = candidate;
            bestRichness = richness;
            bestPrefix = sharedPrefix;
        }
    }

    if (!best.empty()) return best;

    // Fallback to trie prefix suggestions if no one-edit word is valid.
    std::vector<std::string> fallback;
    m_dict.suggestFromPrefix(clean.substr(0, 1), fallback, dct::g_maxSuggest);
    if (!fallback.empty()) return fallback.front();

    return {};
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
			std::cout << "  → " << word << '\n'; 
		}
	}
}
