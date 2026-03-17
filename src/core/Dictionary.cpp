#include "Dictionary.h"
#include "Utils.h"

Dictionary::Dictionary(std::string dbPath) : m_db{ std::move(dbPath) }
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

void Dictionary::suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const 
{
	if (m_trie.isEmpty()) return;
	std::string cleanPrefix{ cleanWord(prefix) };
	if (cleanPrefix.empty()) return;
	m_trie.collectWithPrefix(cleanPrefix, results, limit); // function call

	// shouldn't need this
	// results.erase(std::remove(results.begin(), results.end(), cleanPrefix), results.end());
}

/*********************************
// Dictionary Helper Functions
**********************************/
std::string Dictionary::cleanWord(std::string_view word) const { return dct::sanitizeWord(word); }

void Dictionary::loadTrie() 
{
    sqlite3* sqlDB{ m_db.getDB() };
    sqlite3_stmt* stmt{ nullptr };

    // insert all lemmas
    const char* q1 = "SELECT id, lemma FROM words;";
    if (sqlite3_prepare_v2(sqlDB, q1, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id{ sqlite3_column_int(stmt, 0) };

        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        if (!text) continue;

        std::string lemma{ reinterpret_cast<const char*>(text) };
        m_trie.insert(cleanWord(lemma), id);
    }

    sqlite3_finalize(stmt); // finalize before next preparation


    // insert all forms
    const char* q2 = "SELECT word_id, form FROM forms;";
    if (sqlite3_prepare_v2(sqlDB, q2, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id{ sqlite3_column_int(stmt, 0) };

        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        if (!text) continue;

        std::string form{ reinterpret_cast<const char*>(text) };
        m_trie.insert(cleanWord(form), id);
    }

    sqlite3_finalize(stmt);
}
