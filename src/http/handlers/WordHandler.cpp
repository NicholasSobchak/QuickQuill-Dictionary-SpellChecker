#include "http/handlers/WordHandler.h"
#include "Dictionary.h"
#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

namespace http
{
	namespace
	{
		Dictionary& dict()
		{
			static Dictionary instance;
			return instance;
		}
	}

	std::string search(const std::string& word)
	{
		WordInfo info = dict().getWordInfo(word);
		if (info.lemma.empty())
		{
			nlohmann::json body = {
				{"error", "word not found"},
				{"word", word}
			};
			return body.dump();
		}

		return toWordJson(info);
	}
}
