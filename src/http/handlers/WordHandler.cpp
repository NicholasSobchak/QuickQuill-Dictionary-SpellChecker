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

		// this clears up any weird rendering that might come from certain characters
		// Decode percent-encoded user input from the path segment (handles %HH and + for space).
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

	// returns (JSON body, status)
	SearchResult search(const std::string& word)
	{
		const std::string decoded = decodeInput(word); // correct input
		if (decoded.empty()) // strictly prohibited characters
		{
			nlohmann::json body = {
				{"error", "Enter a word"}
			};
			return { body.dump(), 400 };
		}

		// determine if there are acceptable characters
		const bool allowedChars = std::all_of(decoded.begin(), decoded.end(), [](unsigned char c) {
			return std::isalpha(c) || c == '\'' || c == '-' || c == ' ';
		});

		// invalid input
		const std::string sanitized = dct::sanitizeWord(decoded);
		if (decoded.empty() || !allowedChars || sanitized.empty())
		{
			nlohmann::json body = {
				{"error", "Enter a word"}
			};
			return { body.dump(), 400 };
		}

		// search for input in the dictionary database
		WordInfo info = dict().getWordInfo(sanitized);
		if (info.lemma.empty())
		{
			nlohmann::json body = {
				{"error", "Word not found"}
			};
			return { body.dump(), 404 };
		}

		return { toWordJson(info, decoded), 200 };
	}

	void warmupDictionary()
	{
		// forces static Dictionary to construct and touches DB
		dict().getWordInfo("warmup"); // or any common word
	}
}
