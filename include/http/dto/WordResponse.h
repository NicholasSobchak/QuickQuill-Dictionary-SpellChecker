#ifndef HTTP_DTO_WORDRESPONSE_H
#define HTTP_DTO_WORDRESPONSE_H

#include "WordInfo.h"
#include <string>

namespace http
{
// TODO: Map WordInfo to JSON response payload.
std::string toWordJson(const WordInfo& info);
}

#endif
