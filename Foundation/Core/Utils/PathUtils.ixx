export module Core.Utils:PathUtils;

import std;

namespace Core
{
    // path에서 거슬러 올라가며 markFile이 있는 폴더를 찾는다. 파일 시스템 오류는 "없음"으로 다룬다
    std::optional<std::filesystem::path> FindWorkspaceRootByMarkFile(
        std::filesystem::path path,
        const std::filesystem::path& markFile)
    {
        std::error_code error;
        while (true)
        {
            if (std::filesystem::exists(path / markFile, error))
                return path;

            const auto parent = path.parent_path();
            if (parent == path)
                return std::nullopt;

            path = parent;
        }
    }

    // startPath(파일이나 폴더)에서 거슬러 올라가 markFile이 있는 폴더를 찾는다. 못 찾으면 nullopt이다
    export std::optional<std::filesystem::path> FindRootByMarker(
        const std::filesystem::path& startPath,
        const std::filesystem::path& markFile)
    {
        std::error_code error;
        const auto directory =
            std::filesystem::is_directory(startPath, error)
            ? startPath
            : startPath.parent_path();

        return FindWorkspaceRootByMarkFile(directory, markFile);
    }

    export std::filesystem::path ResolvePath(
        const std::filesystem::path& base,
        const std::filesystem::path& input)
    {
        const auto resolved = input.is_absolute() ? input : base / input;
        return resolved.lexically_normal();
    }

    // 경로와 UTF-8 문자열은 PathFromUtf8/PathToUtf8로 오간다 (구분자는 '/'). 규칙은 path-usage.md에 있다
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