#include "Dictionary.h"
#include "Utils.h"
#include "Config.h"
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>

Dictionary::Dictionary() : m_db{ Config::getInstance().getDatabasePath() }
{
    m_db.createTables();
    loadTrie();
}

WordInfo Dictionary::getWordInfo(std::string_view word) const
{
    WordInfo info;
    std::string clean = cleanWord(word);
    if (clean.empty()) return info;

    dct::WordId id = m_trie.getWordId(clean);
    if (id.value == dct::g_defaultId) return info;

    auto cached = m_cache.find(id.value);
    if (cached != m_cache.end())
    {
        return cached->second;
    }

    info = m_db.getInfo(id);
    m_cache.try_emplace(id.value, info);

    return info;
}

bool Dictionary::contains(std::string_view word) const
{
    std::string clean = cleanWord(word);
    if (clean.empty()) return false;

    dct::WordId id = m_trie.getWordId(clean);
    return id.value != dct::g_defaultId;
}

// implement suggestions based on word length or common words
void Dictionary::suggestFromPrefix(std::string_view prefix, std::vector<std::pair<std::string, dct::Frequency>> &results, std::size_t limit) const 
{
	if (m_trie.isEmpty()) return;
	std::string cleanPrefix{ cleanWord(prefix) };
	if (cleanPrefix.empty()) return;
	m_trie.collectWithPrefix(cleanPrefix, results, limit); // function call

	std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
		return a.second.value > b.second.value;
	});
}

std::vector<std::string> Dictionary::suggestSpelling(std::string_view word) const
{
    std::string clean = cleanWord(word);
    if (m_trie.contains(clean)) 
	{
        return {}; // word is correct, no suggestions needed
    }

    std::unordered_set<std::string> suggestionsSet;
    std::string candidate;

	// deletion
    for (int i = 0; i < clean.length(); ++i) 
	{
        candidate = clean;
        candidate.erase(i, 1);
        if (m_trie.contains(candidate)) {
            suggestionsSet.insert(candidate);
        }
    }

    // transpositions
    for (int i = 0; i < clean.length() - 1; ++i) 
	{
        candidate = clean;
        std::swap(candidate[i], candidate[i + 1]);
        if (m_trie.contains(candidate)) {
            suggestionsSet.insert(candidate);
        }
    }

    // substitutions
    for (int i = 0; i < clean.length(); ++i) 
	{
        candidate = clean;
        for (char c = 'a'; c <= 'z'; ++c) {
            candidate[i] = c;
            if (m_trie.contains(candidate)) {
                suggestionsSet.insert(candidate);
            }
        }
    }

    // insertions
    for (int i = 0; i <= clean.length(); ++i) 
	{
        for (char c = 'a'; c <= 'z'; ++c) {
            candidate = clean;
            candidate.insert(i, 1, c);
            if (m_trie.contains(candidate)) {
                suggestionsSet.insert(candidate);
            }
        }
    }

    std::vector<std::string> suggestions(suggestionsSet.begin(), suggestionsSet.end());

	// rank suggestions based on lambda calling Levenshteins's alorithm
	std::sort(suggestions.begin(), suggestions.end(), [&](const std::string& a, const std::string& b) {
		return dct::calculateLevenshteinDistance(clean, a) < dct::calculateLevenshteinDistance(clean, b);
	});

    return suggestions;
}


/*********************************
// Dictionary Helper Functions
**********************************/
std::string Dictionary::cleanWord(std::string_view word) const { return dct::sanitizeWord(word); }

void Dictionary::loadTrie() 
{
	auto trieLoader = [this](dct::WordId id, std::string_view text, dct::Frequency frequency) 
	{
		m_trie.insert(cleanWord(text), id, frequency);
	};

	// pass to database to retrieve words
	m_db.streamAllWordsAndForms(trieLoader);
}
