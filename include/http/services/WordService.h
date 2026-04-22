#ifndef HTTP_SERVICES_WORDSERVICE_H
#define HTTP_SERVICES_WORDSERVICE_H

#include "Dictionary.h"
#include "SpellChecker.h"

#include <string>

namespace http {
struct SearchResult {
  std::string body;
  int status{200};
};

struct SuggestResult {
  std::string body;
  int status{200};
};

class WordService {
public:
  WordService(Dictionary& dict, SpellChecker& checker);

  SearchResult search(const std::string& word) const;
  SuggestResult suggest(const std::string& word) const;
  void warmupDictionary() const;

private:
  static std::string decodeInput(const std::string& in);

  Dictionary& m_dict;
  SpellChecker& m_checker;
};

WordService& wordService();
void warmupDictionary();
} // namespace http

#endif
