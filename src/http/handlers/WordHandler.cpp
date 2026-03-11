#include "http/handlers/WordHandler.h"
#include "Dictionary.h"
#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

namespace http
{
	namespace // declare internal linkage 
	{
		Dictionary& dict()
		{
			static Dictionary instance; 
			return instance;
		}
	}

	// returns (JSON body, status)
		SearchResult search(const std::string& word)
		{
			const bool allowedChars = std::all_of(word.begin(), word.end(), [](unsigned char c) {
				return std::isalpha(c) || c == '\'' || c == '-';
			});

			if (word.empty() || !allowedChars)
			{
				nlohmann::json body = {
					{"error", "Enter a word"}
				};
				return { body.dump(), 400 };
			}

			WordInfo info = dict().getWordInfo(word);
			if (info.lemma.empty())
			{
				nlohmann::json body = {
					{"error", "Word not found"}
				};
				return { body.dump(), 404 };
			}

			return { toWordJson(info, std::string(word)), 200 };
		}

	void warmupDictionary()
	{
		// forces static Dictionary to construct and touches DB
		dict().getWordInfo("warmup"); // or any common word
	}
}
