#ifndef HTTP_HANDLERS_WORDHANDLER_H
#define HTTP_HANDLERS_WORDHANDLER_H

#include <string>

namespace http
{
	struct SearchResult
	{
		std::string body;
		int status{ 200 };
	};

	// Returns JSON body and HTTP status for a single searched word.
	SearchResult search(const std::string& word);
	void warmupDictionary();
}

#endif
