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
}