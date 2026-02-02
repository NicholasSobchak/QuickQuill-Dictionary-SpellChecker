#ifndef UTILS_H
#define UTLS_H

namespace dct 
{
        inline constexpr const char *g_dictDb {"dictionary.db"}; // inline to avoid linker errors
        inline constexpr int g_alpha {26};
	    inline constexpr int g_maxSuggest {10};

		// add getIndex for c-'a' senarios??
}
#endif
