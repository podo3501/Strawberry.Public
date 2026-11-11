export module Core.Utils:StringOperations;

import std;

namespace Core
{
    // ASCII 영문자만 소문자로 바꾼 복사본이다 (로케일과 무관하다)
    export std::string ToLower(std::string_view s)
    {
        std::string result(s);
        for (char& c : result)
        {
            if (c >= 'A' && c <= 'Z')
                c = static_cast<char>(c + ('a' - 'A'));
        }
        return result;
    }
}
