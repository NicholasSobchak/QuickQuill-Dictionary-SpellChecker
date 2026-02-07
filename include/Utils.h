#ifndef UTILS_H
#define UTILS_H

#include <cctype>
#include <string>
#include <string_view>

namespace dct
{
    inline constexpr std::string_view g_dictDb{ "dictionary.db" }; // inline to avoid linker errors
    inline constexpr int g_alpha{ 26 };
    inline constexpr int g_maxSuggest{ 10 };
    inline constexpr int g_defaultId{ -1 };

    inline std::string sanitizeWord(std::string_view word)
    {
        std::string clean;
        clean.reserve(word.size());

        for (unsigned char c : word)
        {
            if (!std::isalpha(c)) continue;
            clean.push_back(static_cast<char>(std::tolower(c)));
        }

        return clean;
    }
}

#endif
