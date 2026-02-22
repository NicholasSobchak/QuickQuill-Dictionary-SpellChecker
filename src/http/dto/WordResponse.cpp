#include "http/dto/WordResponse.h"
#include "nlohmann/json.hpp"

namespace http
{
	std::string toWordJson(const WordInfo& info)
	{
		// TODO: Convert WordInfo into stable JSON response format.
		nlohmann::json j;
		j["id"] = info.id;
		j["lemma"] = info.lemma;
		j["etymology"] = info.etymology;

		j["forms"] = nlohmann::json::array();
		for (const auto& f : info.forms)
		{
			j["forms"].push_back({{"form", f.form}, {"tag", f.tag}});
		}

		j["senses"] = nlohmann::json::array();
		for (const auto& s : info.senses)
		{
			j["senses"].push_back({
				{"pos", s.pos},
				{"definition", s.definition},
				{"examples", s.examples},
				{"synonyms", s.synonyms},
				{"antonyms", s.antonyms}
			});
		}

		return j.dump();
	}
}
