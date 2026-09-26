export module Core.Utils:StringOperations;

import std;

namespace Core
{
    export void ToLower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }

    export void ToUpper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    }

    export std::string ToLowerCopy(std::string s)
    {
        ToLower(s);
        return s;
    }

    export std::string ToUpperCopy(std::string s)
    {
        ToUpper(s);
        return s;
    }

    export std::string ToLower(std::string_view s)
    {
        std::string result(s);
        ToLower(result);
        return result;
    }

    export std::string ToUpper(std::string_view s)
    {
        std::string result(s);
        ToUpper(result);
        return result;
    }

    export std::string_view GetExtension(std::string_view path)
    {
        std::size_t slashPos = path.find_last_of("/\\");
        std::size_t dotPos = path.find_last_of('.');

        if (dotPos == std::string_view::npos)
            return {};

        if (slashPos != std::string_view::npos && dotPos < slashPos)
        {
            return {};
        }

        return path.substr(dotPos);
    }

    export std::string AppendIfPresent(const std::string& base, const std::string& suffix, const std::string& delim)
    {
        return suffix.empty() ? base : base + delim + suffix;
    }

    static std::pair<std::string_view, std::string_view> SplitNameAndId(std::string_view name)
    {
        auto underscore = name.find('_');
        if (underscore == std::string_view::npos) return { name, {} };

        auto prefix = name.substr(0, underscore);
        auto idStr = name.substr(underscore + 1);
        
        if (idStr.empty() || !std::all_of(idStr.begin(), idStr.end(), [](unsigned char c) {
            return std::isdigit(c);
            }))
            return {};

        return { prefix, idStr };
    }

    export std::pair<std::string, int> ExtractNameAndId(std::string_view name)
    {
        auto [prefix, idStr] = SplitNameAndId(name);
        if (prefix.empty()) return { "", 0 };

        int id = 0;
        if (!idStr.empty())
        {
            std::from_chars(idStr.data(), idStr.data() + idStr.size(), id);
        }

        return { std::string(prefix), id };
    }
}