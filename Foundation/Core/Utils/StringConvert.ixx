export module Core.Utils:StringConvert;

import std;

namespace Core
{
    inline constexpr char32_t kReplacementChar = U'\uFFFD';

    struct Utf8Char
    {
        char32_t codePoint{ kReplacementChar };
        bool valid{ true };   // 깨진 바이트이면 false이고 codePoint는 U+FFFD이다
    };

    // text[offset]에서 UTF-8 한 글자를 읽고 offset을 그만큼 옮긴다. 끝이면 nullopt.
    // 깨진 입력(잘린 시퀀스, 짧게 쓴 인코딩, 서로게이트, U+10FFFF 초과, 잘못된 시작 바이트)은 1바이트만 건너뛰고 U+FFFD를 낸다
    std::optional<Utf8Char> DecodeUtf8Char(std::string_view text, std::size_t& offset) noexcept
    {
        if (offset >= text.size())
            return std::nullopt;

        const auto lead = static_cast<unsigned char>(text[offset]);
        int extra = 0;
        char32_t codePoint = 0;
        char32_t minimum = 0;

        if (lead < 0x80) { ++offset; return Utf8Char{ lead, true }; }
        else if (lead >= 0xC2 && lead <= 0xDF) { extra = 1; codePoint = lead & 0x1F; minimum = 0x80; }
        else if (lead >= 0xE0 && lead <= 0xEF) { extra = 2; codePoint = lead & 0x0F; minimum = 0x800; }
        else if (lead >= 0xF0 && lead <= 0xF4) { extra = 3; codePoint = lead & 0x07; minimum = 0x10000; }
        else { ++offset; return Utf8Char{ kReplacementChar, false }; }

        const auto broken = [&offset]() { ++offset; return Utf8Char{ kReplacementChar, false }; };
        if (offset + extra >= text.size())
            return broken();   // 뒤따르는 바이트가 모자란다

        for (int k = 1; k <= extra; ++k)
        {
            const auto next = static_cast<unsigned char>(text[offset + k]);
            if ((next & 0xC0) != 0x80)
                return broken();
            codePoint = (codePoint << 6) | (next & 0x3F);
        }

        if (codePoint < minimum || codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
            return broken();

        offset += extra + 1;
        return Utf8Char{ codePoint, true };
    }

    // 올바른 UTF-8인가 (짧게 쓴 인코딩, 서로게이트, 범위 밖, 잘린 시퀀스는 올바르지 않다)
    export bool IsValidUtf8(std::string_view text) noexcept
    {
        std::size_t offset = 0;
        while (const auto decoded = DecodeUtf8Char(text, offset))
        {
            if (!decoded->valid)
                return false;
        }
        return true;
    }

    // 깨진 바이트는 1바이트마다 U+FFFD가 되고, NUL 문자도 그대로 남는다
    export std::vector<char32_t> UTF8ToUTF32(std::string_view text)
    {
        std::vector<char32_t> result;
        result.reserve(text.size());

        std::size_t offset = 0;
        while (const auto decoded = DecodeUtf8Char(text, offset))
            result.push_back(decoded->codePoint);

        return result;
    }

    // UTF-16(wchar_t)으로 바꾼다. 깨진 바이트는 1바이트마다 U+FFFD가 된다
    export std::wstring UTF8ToWString(std::string_view text)
    {
        std::wstring result;
        result.reserve(text.size());

        std::size_t offset = 0;
        while (const auto decoded = DecodeUtf8Char(text, offset))
        {
            const char32_t codePoint = decoded->codePoint;
            if (codePoint <= 0xFFFF)
            {
                result.push_back(static_cast<wchar_t>(codePoint));
                continue;
            }

            const char32_t offsetCode = codePoint - 0x10000;   // 서로게이트 쌍
            result.push_back(static_cast<wchar_t>(0xD800 + (offsetCode >> 10)));
            result.push_back(static_cast<wchar_t>(0xDC00 + (offsetCode & 0x3FF)));
        }

        return result;
    }

    // 서로게이트(0xD800~0xDFFF)와 U+10FFFF를 넘는 값은 U+FFFD로 쓴다 (결과가 언제나 올바른 UTF-8이다)
    export void UTF32ToUTF8Char(char32_t c, std::string& out)
    {
        std::uint32_t codePoint = static_cast<std::uint32_t>(c);
        if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
            codePoint = kReplacementChar;

        if (codePoint <= 0x7F)
        {
            out.push_back(static_cast<char>(codePoint));
        }
        else if (codePoint <= 0x7FF)
        {
            out.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
            out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
        else if (codePoint <= 0xFFFF)
        {
            out.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
            out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
        else
        {
            out.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
            out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
    }

    // UTF-16(wchar_t)의 서로게이트 쌍은 하나의 문자로 합쳐 UTF-8로 쓴다. 짝이 없는 서로게이트는 U+FFFD가 된다
    export std::string WStringToUTF8(std::wstring_view text)
    {
        std::string result;
        result.reserve(text.size() * 3);

        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const auto unit = static_cast<std::uint32_t>(text[i]);
            const bool highSurrogate = unit >= 0xD800 && unit <= 0xDBFF;

            if (highSurrogate && i + 1 < text.size())
            {
                const auto low = static_cast<std::uint32_t>(text[i + 1]);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    UTF32ToUTF8Char(static_cast<char32_t>(0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00)), result);
                    ++i;
                    continue;
                }
            }

            UTF32ToUTF8Char(static_cast<char32_t>(unit), result);
        }

        return result;
    }
}
