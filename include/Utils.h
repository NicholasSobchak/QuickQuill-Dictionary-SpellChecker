#ifndef UTILS_H
#define UTLS_H
#include <string_view>

namespace dct 
{
        inline constexpr std::string_view g_dictDb{ "dictionary.db" }; // inline to avoid linker errors
        inline constexpr int g_alpha{ 26 };
	    inline constexpr int g_maxSuggest{ 10 };
	    inline constexpr int g_defaultId{ -1 };

		// add getIndex for c-'a' senarios??
}
#endif
