#include "core/Dictionary.h"

#include "app/Config.h"
#include "dct/dct.h"
#include "random.h"

struct Dictionary::ThreadResources
{
  explicit ThreadResources(const std::string &dbPath) : db{dbPath} {}

  Database db;
  std::unordered_map<int, WordInfo> cache;
};

Dictionary::Dictionary() : m_dbPath{Config::getInstance().getDatabasePath()}
{
  // Ensure DB schema exists and populate trie using a short-lived connection.
  Database bootstrap{m_dbPath};
  bootstrap.createTables();
  loadTrie();
}

Dictionary::ThreadResources &Dictionary::resources() const
{
  thread_local ThreadResources resources{m_dbPath};
  return resources;
}

Database &Dictionary::db() const { return resources().db; }

std::unordered_map<int, WordInfo> &Dictionary::cache() const { return resources().cache; }

WordInfo Dictionary::getWordInfo(std::string_view word) const
{
  WordInfo info;
  std::string clean{cleanWord(word)};
  if (clean.empty())
  {
    return info;
  }

  dct::WordId id = m_trie.getWordId(clean);
  if (id.value == dct::g_defaultId)
  {
    return info;
  }

  auto &localCache = cache();
  auto cached = localCache.find(id.value);
  if (cached != localCache.end())
  {
    return cached->second;
  }

  info = db().getInfo(id);
  localCache.try_emplace(id.value, info);

  return info;
}

std::vector<std::string>
Dictionary::getAlternativeSearches(std::string_view word, dct::WordId currentId) const
{
  std::vector<std::string> alternatives;
  std::string clean{cleanWord(word)};
  if (clean.empty())
  {
    return alternatives;
  }

  const auto ids = db().findMatchingWordIds(clean);
  if (ids.size() <= 1)
  {
    return alternatives;
  }

  std::unordered_set<std::string> seen;
  auto &localCache = cache();
  for (const auto &id : ids)
  {
    if (currentId.value != dct::g_defaultId && id.value == currentId.value)
    {
      continue;
    }

    WordInfo info;
    auto cached = localCache.find(id.value);
    if (cached != localCache.end())
    {
      info = cached->second;
    }
    else
    {
      info = db().getInfo(id);
      localCache.try_emplace(id.value, info);
    }

    const std::string label = info.displayLemma.empty() ? info.lemma : info.displayLemma;
    if (label.empty())
    {
      continue;
    }

    const std::string normalized = cleanWord(label);
    if (normalized.empty() || !seen.insert(normalized).second)
    {
      continue;
    }
    alternatives.push_back(label);
  }

  return alternatives;
}

std::vector<std::string> Dictionary::suggestSynonyms(std::string_view word) const
{
  std::vector<std::string> synonymSuggestions;
  std::string clean{cleanWord(word)};
  if (clean.empty())
  {
    return synonymSuggestions;
  }

  dct::WordId id = m_trie.getWordId(clean);
  if (id.value == dct::g_defaultId)
  {
    return synonymSuggestions;
  }

  WordInfo info;
  auto &localCache = cache();
  auto cached = localCache.find(id.value);
  if (cached != localCache.end())
  {
    info = cached->second;
  }
  else
  {
    info = db().getInfo(id);
    localCache.try_emplace(id.value, info);
  }

  // create pool
  std::vector<std::string> pool;
  for (const auto &s : info.senses)
  {
    for (const auto &syn : s.synonyms)
    {
      pool.push_back(syn);
    }
  }

  if (pool.empty())
  {
    return synonymSuggestions;
  }

  const auto pick = Random::get<std::size_t>(0, pool.size() - 1);
  std::size_t count{std::min(pool.size(), std::size_t{pick})};
  while (synonymSuggestions.size() < count)
  {
    const auto index = Random::get<std::size_t>(0, pool.size() - 1);
    const auto &pick = pool[index];
    // only add if not already picked
    if (std::find(synonymSuggestions.begin(), synonymSuggestions.end(), pick) ==
        synonymSuggestions.end())
    {
      synonymSuggestions.push_back(pick);
    }
  }
  return synonymSuggestions;
}

bool Dictionary::contains(std::string_view word) const
{
  std::string clean = cleanWord(word);
  if (clean.empty())
  {
    return false;
  }

  dct::WordId id = m_trie.getWordId(clean);
  return id.value != dct::g_defaultId;
}

std::vector<std::string> Dictionary::suggestFromPrefix(std::string_view word) const
{
  // std::string prefix{ m_trie.getPrefix(word) };

  std::string clean{cleanWord(word)};
  if (clean.empty())
  {
    return {};
  }

  std::unordered_set<std::string> suggestionsSet = collectSuggestedWords(clean);
  std::vector<std::string> suggestions(suggestionsSet.begin(), suggestionsSet.end());

  suggestions.erase(std::remove(suggestions.begin(), suggestions.end(), clean), suggestions.end());
  // rank suggestions based on lambda calling Levenshteins's algorithm
  std::sort(
      suggestions.begin(), suggestions.end(),
      [&](const std::string &a, const std::string &b)
      {
        return dct::calculateLevenshteinDistance(clean, a) <
               dct::calculateLevenshteinDistance(clean, b);
      });

  return suggestions;
}

std::vector<std::string> Dictionary::suggestSpelling(std::string_view word) const
{
  std::string clean{cleanWord(word)};
  if (clean.empty())
  {
    return {};
  }
  if (m_trie.contains(clean))
  {
    return {}; // word is correct, no suggestions needed
  }

  std::unordered_set<std::string> suggestionsSet = collectSuggestedWords(clean);
  std::vector<std::string> suggestions(suggestionsSet.begin(), suggestionsSet.end());

  // rank suggestions based on lambda calling Levenshteins's alorithm
  std::sort(
      suggestions.begin(), suggestions.end(),
      [&](const std::string &a, const std::string &b)
      {
        return dct::calculateLevenshteinDistance(clean, a) <
               dct::calculateLevenshteinDistance(clean, b);
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
  { m_trie.insert(cleanWord(text), id, frequency); };

  // pass to database to retrieve words
  Database loaderDb{m_dbPath};
  loaderDb.createTables();
  loaderDb.streamAllWordsAndForms(trieLoader);
}

std::unordered_set<std::string> Dictionary::collectSuggestedWords(std::string_view word) const
{
  // for safety
  std::string clean{cleanWord(word)};

  std::unordered_set<std::string> suggestionsSet;
  std::string candidate;

  // deletion
  for (int i = 0; i < clean.length(); ++i)
  {
    candidate = clean;
    candidate.erase(i, 1);
    if (m_trie.contains(candidate))
    {
      suggestionsSet.insert(candidate);
    }
  }

  // transpositions
  for (int i = 0; i < clean.length() - 1; ++i)
  {
    candidate = clean;
    std::swap(candidate[i], candidate[i + 1]);
    if (m_trie.contains(candidate))
    {
      suggestionsSet.insert(candidate);
    }
  }

  // substitutions
  for (int i = 0; i < clean.length(); ++i)
  {
    candidate = clean;
    for (char c = 'a'; c <= 'z'; ++c)
    {
      candidate[i] = c;
      if (m_trie.contains(candidate))
      {
        suggestionsSet.insert(candidate);
      }
    }
  }

  // insertions
  for (int i = 0; i <= clean.length(); ++i)
  {
    for (char c = 'a'; c <= 'z'; ++c)
    {
      candidate = clean;
      candidate.insert(i, 1, c);
      if (m_trie.contains(candidate))
      {
        suggestionsSet.insert(candidate);
      }
    }
  }

  return suggestionsSet;
}
