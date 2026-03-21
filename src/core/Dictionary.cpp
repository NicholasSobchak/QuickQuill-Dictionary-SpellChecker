#include "Dictionary.h"
#include "Utils.h"
#include <cstdlib> // For getenv
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>

// Helper function to get DB path from environment variable or use default
std::string getDatabasePath(const std::string& defaultPath) {
    if (const char* envPath = std::getenv("QUICKQUILL_DB_PATH")) {
        return std::string(envPath);
    }
    return defaultPath;
}

Dictionary::Dictionary(const std::string& dbPath) : m_db{ getDatabasePath(dbPath) }
{
    m_db.createTables();
    loadTrie();
}

WordInfo Dictionary::getWordInfo(std::string_view word) const
{
    WordInfo info;
    std::string clean = cleanWord(word);
    if (clean.empty()) return info;

    int id = m_trie.getWordId(clean);
    if (id == dct::g_defaultId) return info;

    auto cached = m_cache.find(id);
    if (cached != m_cache.end())
    {
        return cached->second;
    }

    info = m_db.getInfo(id);
    m_cache.try_emplace(id, info);

    return info;
}

bool Dictionary::contains(std::string_view word) const
{
    std::string clean = cleanWord(word);
    if (clean.empty()) return false;

    int id = m_trie.getWordId(clean);
    return id != dct::g_defaultId;
}

// implement suggestions based on word length or common words
void Dictionary::suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const 
{
	if (m_trie.isEmpty()) return;
	std::string cleanPrefix{ cleanWord(prefix) };
	if (cleanPrefix.empty()) return;
	m_trie.collectWithPrefix(cleanPrefix, results, limit); // function call

	// shouldn't need this
	// results.erase(std::remove(results.begin(), results.end(), cleanPrefix), results.end());
}

std::vector<std::string> Dictionary::suggestSpelling(std::string_view word) const
{
    std::string clean_word = cleanWord(word);
    if (m_trie.contains(clean_word)) {
        return {}; // Word is correct, no suggestions needed
    }

    std::unordered_set<std::string> suggestions_set;
    std::string candidate;

	// deletion
    for (int i = 0; i < clean_word.length(); ++i) {
        candidate = clean_word;
        candidate.erase(i, 1);
        if (m_trie.contains(candidate)) {
            suggestions_set.insert(candidate);
        }
    }

    // transpositions
    for (int i = 0; i < clean_word.length() - 1; ++i) {
        candidate = clean_word;
        std::swap(candidate[i], candidate[i + 1]);
        if (m_trie.contains(candidate)) {
            suggestions_set.insert(candidate);
        }
    }

    // substitutions
    for (int i = 0; i < clean_word.length(); ++i) {
        candidate = clean_word;
        for (char c = 'a'; c <= 'z'; ++c) {
            candidate[i] = c;
            if (m_trie.contains(candidate)) {
                suggestions_set.insert(candidate);
            }
        }
    }

    // insertions
    for (int i = 0; i <= clean_word.length(); ++i) {
        for (char c = 'a'; c <= 'z'; ++c) {
            candidate = clean_word;
            candidate.insert(i, 1, c);
            if (m_trie.contains(candidate)) {
                suggestions_set.insert(candidate);
            }
        }
    }

    std::vector<std::string> suggestions(suggestions_set.begin(), suggestions_set.end());
    std::sort(suggestions.begin(), suggestions.end()); // For consistent order
    return suggestions;
}


/*********************************
// Dictionary Helper Functions
**********************************/
std::string Dictionary::cleanWord(std::string_view word) const { return dct::sanitizeWord(word); }

void Dictionary::loadTrie() 
{
	auto trieLoader = [this](int id, std::string_view text) 
	{
		m_trie.insert(cleanWord(text), id);
	};

	// pass to database to retrieve words
	m_db.streamAllWordsAndForms(trieLoader);
}
