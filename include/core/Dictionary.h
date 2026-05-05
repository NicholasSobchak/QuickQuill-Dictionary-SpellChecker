#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "core/Trie.h"
#include "data/Database.h"
#include "data/RedisCache.h"
#include "dct/WordInfo.h"

class Dictionary
{
public:
  explicit Dictionary();
  ~Dictionary() = default;

  WordInfo getWordInfo(std::string_view word) const;
  std::vector<std::string> getAlternativeSearches(
      std::string_view word, dct::WordId currentId = dct::WordId{dct::g_defaultId}) const;
  std::vector<std::string> suggestSynonyms(std::string_view word) const;
  bool contains(std::string_view word) const;

  // spellchecking functions
  std::vector<std::string> suggestFromPrefix(std::string_view word) const;
  std::vector<std::string> suggestSpelling(std::string_view word) const;

private:
  Trie m_trie;
  std::string m_dbPath;

  std::string cleanWord(std::string_view word) const;
  void loadTrie(); // populate Trie using Database
  std::unordered_set<std::string> collectSuggestedWords(std::string_view word) const;

  // Each request thread gets its own sqlite connection (Database) and Redis cache
  Database &db() const;
  RedisCache &redisCache() const;

  struct ThreadResources;
  ThreadResources &resources() const;
};
#endif
