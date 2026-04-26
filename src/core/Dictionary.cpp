#include "core/Dictionary.h"

#include "app/Config.h"
#include "dct/dct.h"
#include "random.h"

Dictionary::Dictionary() : m_db{Config::getInstance().getDatabasePath()} {
  m_db.createTables();
  loadTrie();
}

WordInfo Dictionary::getWordInfo(std::string_view word) const {
  WordInfo info;
  std::string clean{cleanWord(word)};
  if (clean.empty())
    return info;

  dct::WordId id = m_trie.getWordId(clean);
  if (id.value == dct::g_defaultId)
    return info;

  auto cached = m_cache.find(id.value);
  if (cached != m_cache.end()) {
    return cached->second;
  }

  info = m_db.getInfo(id);
  m_cache.try_emplace(id.value, info);

  return info;
}

std::vector<std::string>
Dictionary::getAlternativeSearches(std::string_view word,
                                   dct::WordId currentId) const {
  std::vector<std::string> alternatives;
  std::string clean{cleanWord(word)};
  if (clean.empty())
    return alternatives;

  const auto ids = m_db.findMatchingWordIds(clean);
  if (ids.size() <= 1)
    return alternatives;

  std::unordered_set<std::string> seen;
  for (const auto &id : ids) {
    if (currentId.value != dct::g_defaultId && id.value == currentId.value)
      continue;

    WordInfo info;
    auto cached = m_cache.find(id.value);
    if (cached != m_cache.end()) {
      info = cached->second;
    } else {
      info = m_db.getInfo(id);
      m_cache.try_emplace(id.value, info);
    }

    const std::string label =
        info.displayLemma.empty() ? info.lemma : info.displayLemma;
    if (label.empty())
      continue;

    const std::string normalized = cleanWord(label);
    if (normalized.empty() || !seen.insert(normalized).second)
      continue;
    alternatives.push_back(label);
  }

  return alternatives;
}

std::vector<std::string>
Dictionary::suggestSynonyms(std::string_view word) const {
  std::vector<std::string> synonymSuggestions;
  std::string clean{cleanWord(word)};
  if (clean.empty())
    return synonymSuggestions;

  const WordInfo info{getWordInfo(clean)};
  if (!info.senses.empty()) {
    for (std::size_t i{0}; i < info.senses.size(); ++i) {
      int count{};
      const auto &s = info.senses[i];
      if (!s.synonyms.empty()) {
        const auto pick = Random::get<std::size_t>(
            0, s.synonyms.size() - 1); // fix: pick can be 0
        for (std::size_t j{0}; j < s.synonyms.size(); ++j) {
          if (count == pick)
            break;
          const auto index = Random::get<std::size_t>(
              0,
              s.synonyms.size() -
                  1); // remove the synonym so its not part of the draw anymore
          synonymSuggestions.push_back(s.synonyms[index]);
          count++;
        }
      }
    }
  }
  return synonymSuggestions;
}

bool Dictionary::contains(std::string_view word) const {
  std::string clean = cleanWord(word);
  if (clean.empty())
    return false;

  dct::WordId id = m_trie.getWordId(clean);
  return id.value != dct::g_defaultId;
}

std::vector<std::string>
Dictionary::suggestFromPrefix(std::string_view word) const {
  // std::string prefix{ m_trie.getPrefix(word) };

  std::string clean{cleanWord(word)};
  if (clean.empty()) {
    return {};
  }

  std::unordered_set<std::string> suggestionsSet = collectSuggestedWords(clean);
  std::vector<std::string> suggestions(suggestionsSet.begin(),
                                       suggestionsSet.end());

  suggestions.erase(std::remove(suggestions.begin(), suggestions.end(), clean),
                    suggestions.end());
  // rank suggestions based on lambda calling Levenshteins's algorithm
  std::sort(suggestions.begin(), suggestions.end(),
            [&](const std::string &a, const std::string &b) {
              return dct::calculateLevenshteinDistance(clean, a) <
                     dct::calculateLevenshteinDistance(clean, b);
            });

  return suggestions;
}

std::vector<std::string>
Dictionary::suggestSpelling(std::string_view word) const {
  std::string clean{cleanWord(word)};
  if (clean.empty())
    return {};
  if (m_trie.contains(clean)) {
    return {}; // word is correct, no suggestions needed
  }

  std::unordered_set<std::string> suggestionsSet = collectSuggestedWords(clean);
  std::vector<std::string> suggestions(suggestionsSet.begin(),
                                       suggestionsSet.end());

  // rank suggestions based on lambda calling Levenshteins's alorithm
  std::sort(suggestions.begin(), suggestions.end(),
            [&](const std::string &a, const std::string &b) {
              return dct::calculateLevenshteinDistance(clean, a) <
                     dct::calculateLevenshteinDistance(clean, b);
            });

  return suggestions;
}

/*********************************
// Dictionary Helper Functions
**********************************/
std::string Dictionary::cleanWord(std::string_view word) const {
  return dct::sanitizeWord(word);
}

void Dictionary::loadTrie() {
  auto trieLoader = [this](dct::WordId id, std::string_view text,
                           dct::Frequency frequency) {
    m_trie.insert(cleanWord(text), id, frequency);
  };

  // pass to database to retrieve words
  m_db.streamAllWordsAndForms(trieLoader);
}

std::unordered_set<std::string>
Dictionary::collectSuggestedWords(std::string_view word) const {
  // for safety
  std::string clean{cleanWord(word)};

  std::unordered_set<std::string> suggestionsSet;
  std::string candidate;

  // deletion
  for (int i = 0; i < clean.length(); ++i) {
    candidate = clean;
    candidate.erase(i, 1);
    if (m_trie.contains(candidate)) {
      suggestionsSet.insert(candidate);
    }
  }

  // transpositions
  for (int i = 0; i < clean.length() - 1; ++i) {
    candidate = clean;
    std::swap(candidate[i], candidate[i + 1]);
    if (m_trie.contains(candidate)) {
      suggestionsSet.insert(candidate);
    }
  }

  // substitutions
  for (int i = 0; i < clean.length(); ++i) {
    candidate = clean;
    for (char c = 'a'; c <= 'z'; ++c) {
      candidate[i] = c;
      if (m_trie.contains(candidate)) {
        suggestionsSet.insert(candidate);
      }
    }
  }

  // insertions
  for (int i = 0; i <= clean.length(); ++i) {
    for (char c = 'a'; c <= 'z'; ++c) {
      candidate = clean;
      candidate.insert(i, 1, c);
      if (m_trie.contains(candidate)) {
        suggestionsSet.insert(candidate);
      }
    }
  }

  return suggestionsSet;
}
