#include "http/handlers/WordHandler.h"
#include "SpellChecker.h"
#include "Dictionary.h"
#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

namespace http
{
	namespace 
	{
		/**
		 * Singleton-style access for access safety (any reference to Dictionary and Spellchecker 
		 * will return one instance) 
		 */
		Dictionary& dict()
		{
			static Dictionary instance;
			return instance;
		}

		SpellChecker& checker()
		{
			static SpellChecker instance{ dict() };
			return instance;
		}

		/**
		 * This is a dirty function that just clears up any weird rendering that might come from 
		 * certain characters
		 * 
		 * Decode percent-encoded user input from the path segment (handles %HH and + for space).
		 */
		std::string decodeInput(const std::string& in)
		{
			std::string out;
			out.reserve(in.size());
			for (size_t i = 0; i < in.size(); ++i)
			{
				if (in[i] == '%') // return empty string
				{
					if (i + 2 >= in.size()) return "";
					auto hex = in.substr(i + 1, 2);
					char* end = nullptr;
					long val = std::strtol(hex.c_str(), &end, 16);
					if (end != hex.c_str() + 2) return "";
					out.push_back(static_cast<char>(val));
					i += 2;
				}
				else if (in[i] == '+') // return a space
				{
					out.push_back(' ');
				}
				else // return correct character
				{
					out.push_back(in[i]);
				}
			}
			return out;
		}
	}

	/**
	 * Forces static Dictionary to construct and touches DB
	 */	
	void warmupDictionary()
	{
		dict().getWordInfo("warmup"); // or any common word
	}
	
	/**
	 * Search queries the database or pulls from cache memory using getWordInfo and 
	 * returns (JSON body, status)
	 */
	SearchResult search(const std::string& word)
	{
		const std::string decoded = decodeInput(word); // correct input
		
		// strictly prohibited characters
		if (decoded.empty()) 
		{
			nlohmann::json body = {
				{"error", "Enter a valid word"}
			};
			return { body.dump(), 400 };
		}

		// determine if there are acceptable characters
		const bool allowedChars = std::all_of(decoded.begin(), decoded.end(), [](unsigned char c) {
			return std::isalpha(c) || c == '\'' || c == '-' || c == ' ' || c == '.';
		});

		// invalid input
		const std::string sanitized = dct::sanitizeWord(decoded);
		if (decoded.empty() || !allowedChars || sanitized.empty())
		{
			nlohmann::json body = {
				{"error", "Enter a valid word"}
			};
			return { body.dump(), 400 };
		}

		// search for completely sanitized input in the dictionary database
		WordInfo info = dict().getWordInfo(sanitized);
		if (info.lemma.empty())
		{
			const std::string correctWord = checker().correct(sanitized);
			nlohmann::json body = {
				{"query", sanitized},
				{"found", false},
				{"suggestion", correctWord}
			};
			return { body.dump(), 404 };
		}

		return { toWordJson(info, decoded), 200 };
	}

	SuggestResult suggest(const std::string& word)
	{
		const std::string decoded = decodeInput(word);
		if (decoded.empty())
		{
			return { "[]", 200 };
		}

		const std::string sanitized = dct::sanitizeWord(decoded);
		if (sanitized.empty())
		{
			return { "[]", 200 };
		}

		std::vector<std::string> suggestions = checker().suggest(sanitized);
		nlohmann::json body = suggestions;
		return { body.dump(), 200 };
	}
}
