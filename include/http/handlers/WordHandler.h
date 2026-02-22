#ifndef HTTP_HANDLERS_WORDHANDLER_H
#define HTTP_HANDLERS_WORDHANDLER_H

#include <string>

namespace http
{
	// Returns full dictionary entry JSON for a single searched word.
	std::string search(const std::string& word);
}

#endif
