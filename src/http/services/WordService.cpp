#include "http/services/WordService.h"

#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>

namespace http
{
namespace
{
Dictionary &dict()
{
  static Dictionary instance;
  return instance;
}

SpellChecker &checker()
{
  static SpellChecker instance{dict()};
  return instance;
}
} // namespace

WordService::WordService(Dictionary &dict, SpellChecker &checker) : m_dict{dict}, m_checker{checker}
{
}

WordService &wordService()
{
  static WordService instance{dict(), checker()};
  return instance;
}

/*
 * Forces static dictionary to construct and touch DB/Redis on current thread
 */
void WordService::warmupDictionary() const
{
  static constexpr std::array<std::string_view, 8> kWarmupCandidates = {
      "had", "half", "impaired", "this", "nameless", "grace", "that", "waves"};

  for (const auto candidate : kWarmupCandidates)
  {
    if (!m_dict.contains(candidate))
    {
      continue;
    }

    const WordInfo info = m_dict.getWordInfo(candidate);
    if (info.lemma.empty())
    {
      continue;
    }

    m_dict.getAlternativeSearches(candidate, info.id);
    m_dict.suggestSynonyms(candidate);
  }
}

SearchResult WordService::search(const std::string &word) const
{
  const std::string sanitized = dct::sanitizeWord(word);
  if (sanitized.empty())
  {
    nlohmann::json body = {{"error", "Enter a valid word"}};
    return {body.dump(), 400};
  }

  const bool allowedChars = std::all_of(
      word.begin(), word.end(),
      [](unsigned char c)
      { return std::isalpha(c) || c == '\'' || c == '-' || c == ' ' || c == '.'; });

  if (!allowedChars)
  {
    nlohmann::json body = {{"error", "Enter a valid word"}};
    return {body.dump(), 400};
  }

  WordInfo info = m_dict.getWordInfo(sanitized);
  if (info.lemma.empty())
  {
    // Multi-word query: try individual words
    if (word.find(' ') != std::string::npos)
    {
      std::istringstream iss(word);
      std::vector<std::string> parts;
      std::string part;
      while (iss >> part)
      {
        std::string s = dct::sanitizeWord(part);
        if (!s.empty() && m_dict.contains(s))
        {
          parts.push_back(s);
        }
      }
      if (!parts.empty())
      {
        info = m_dict.getWordInfo(parts[0]);
        if (!info.lemma.empty())
        {
          return {toWordJson(info, word, {}), 200};
        }
      }
    }

    const std::string correctWord = m_checker.correct(sanitized);
    nlohmann::json body = {{"query", sanitized}, {"found", false}, {"suggestion", correctWord}};
    return {body.dump(), 404};
  }

  // alternative searches comes from words with the same id's (same lemmas)
  const auto alternativeSearches = m_dict.getAlternativeSearches(sanitized, info.id);
  return {toWordJson(info, word, alternativeSearches), 200};
}

/*
 * Provides similar searches using the suggest function
 */
SuggestResult WordService::suggest(const std::string &word) const
{
  const std::string sanitized = dct::sanitizeWord(word);
  if (sanitized.empty())
  {
    return {"[]", 200};
  }

  std::vector<std::string> suggestions = m_checker.suggest(sanitized);
  nlohmann::json body = suggestions;
  return {body.dump(), 200};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
AutofillResult WordService::autofill(
    const std::string &prefix,
    const std::vector<std::string> &history,
    const std::vector<std::string> &suggested) const
{
  const std::string sanitized = dct::sanitizeWord(prefix);
  if (sanitized.empty())
  {
    nlohmann::json body = {{"completion", ""}};
    return {body.dump(), 200};
  }

  std::string completion = m_checker.autofill(sanitized, history, suggested);
  nlohmann::json body = {{"completion", completion}};
  return {body.dump(), 200};
}

SuggestSynonymResult WordService::suggestSynonym(const std::string &word) const
{
  const std::string sanitized = dct::sanitizeWord(word);
  if (sanitized.empty())
  {
    return {"[]", 200};
  }

  std::vector<std::string> synonyms = m_dict.suggestSynonyms(sanitized);
  nlohmann::json body = synonyms;
  return {body.dump(), 200};
}
} // namespace http
