#ifndef HTTP_HANDLERS_WORDHANDLER_H
#define HTTP_HANDLERS_WORDHANDLER_H

#include <string>
#include <vector>

namespace http
{
	struct SearchResult
	{
		std::string body;
		int status{ 200 };
	};

	struct SuggestResult
	{
		std::string body;
		int status{ 200 };
	};

	// Returns JSON body and HTTP status for a single searched word.
	SearchResult search(const std::string& word);
	SuggestResult suggest(const std::string& word);
	void warmupDictionary();
}

#endif
