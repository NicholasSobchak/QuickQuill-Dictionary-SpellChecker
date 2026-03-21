#ifndef UTILS_H
#define UTILS_H
#include <string_view>
#include <string>

namespace dct
{
    inline constexpr int g_alpha{ 26 };
    inline constexpr int g_defaultId{ -1 };
    inline constexpr int g_max_suggestions{ 10 };

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
