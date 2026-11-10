export module Core.Utils:Path;

import std;

namespace Core
{
    std::filesystem::path FindWorkspaceRootByMarkFile(
        std::filesystem::path path,
        const std::filesystem::path& markFile)
    {
        while (true)
        {
            if (std::filesystem::exists(path / markFile))
                return path;

            const auto parent = path.parent_path();
            if (parent == path)
                return {};

            path = parent;
        }
    }

    export std::filesystem::path FindRootByMarker(
        const std::filesystem::path& startPath,
        const std::filesystem::path& markFile)
    {
        const auto directory =
            std::filesystem::is_directory(startPath)
            ? startPath
            : startPath.parent_path();

        return FindWorkspaceRootByMarkFile(directory, markFile);
    }

    export std::filesystem::path ResolvePath(
        const std::filesystem::path& base,
        const std::filesystem::path& input) noexcept
    {
        const auto resolved = input.is_absolute() ? input : base / input;
        return resolved.lexically_normal();
    }

    // 소스와 에셋 경로는 UTF-8이다. std::filesystem::path의 narrow 변환은 시스템 코드 페이지를 쓰므로 한글 이름이 깨지거나 예외가 난다.
    // 경로와 문자열을 오갈 때는 PathFromUtf8/PathToUtf8을 쓴다 (PathToUtf8의 구분자는 '/'). PathFromUtf8에는 IsValidUtf8인 문자열만 넘긴다
    export bool IsValidUtf8(std::string_view text) noexcept
    {
        std::size_t i = 0;
        while (i < text.size())
        {
            const auto lead = static_cast<unsigned char>(text[i]);
            int extra = 0;
            char32_t codePoint = 0;
            char32_t minimum = 0;

            if (lead < 0x80) { codePoint = lead; }
            else if (lead >= 0xC2 && lead <= 0xDF) { extra = 1; codePoint = lead & 0x1F; minimum = 0x80; }
            else if (lead >= 0xE0 && lead <= 0xEF) { extra = 2; codePoint = lead & 0x0F; minimum = 0x800; }
            else if (lead >= 0xF0 && lead <= 0xF4) { extra = 3; codePoint = lead & 0x07; minimum = 0x10000; }
            else return false;

            if (i + extra >= text.size())
                return false;   // 뒤따르는 바이트가 모자란다

            for (int k = 1; k <= extra; ++k)
            {
                const auto next = static_cast<unsigned char>(text[i + k]);
                if ((next & 0xC0) != 0x80)
                    return false;
                codePoint = (codePoint << 6) | (next & 0x3F);
            }

            if (codePoint < minimum || codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
                return false;   // 짧게 쓴 인코딩, 범위 밖, 서로게이트는 올바르지 않다

            i += extra + 1;
        }
        return true;
    }

    export std::filesystem::path PathFromUtf8(std::string_view utf8)
    {
        return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
    }

    export std::string PathToUtf8(const std::filesystem::path& path)
    {
        const std::u8string text = path.generic_u8string();
        return std::string(reinterpret_cast<const char*>(text.data()), text.size());
    }
}