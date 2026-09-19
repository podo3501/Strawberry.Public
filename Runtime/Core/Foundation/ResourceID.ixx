module;

#include "Preprocessor/Assert.h"

export module Core.ResourceID;

import std;
import Core.StringUtils;

namespace
{
    constexpr std::string_view kBuiltinPrefix = "builtin://";
    constexpr std::string_view kRuntimePrefix = "runtime://";
    constexpr std::string_view kPathPrefix = "path://";

    // 필요 시 내부에서 사용하는 간단한 경로 정규화 함수
    std::string NormalizePath(const std::filesystem::path& path)
    {
        std::string s = path.lexically_normal().generic_string();
        return Core::ToLowerCopy(s);
    }
}

namespace Core
{
    export enum class ResourceIDType
    {
        Invalid,
        Builtin,
        Runtime,
        Path,
    };

    export class ResourceID
    {
    public:
        ResourceID() = default;
        auto operator<=>(const ResourceID&) const = default;

        static ResourceID MakePath(std::string_view path)
        {
            std::filesystem::path fsPath(path);

            Assert(fsPath.has_extension());
            return ResourceID(std::string(kPathPrefix) + NormalizePath(fsPath));
        }

        static ResourceID MakeRuntime(std::string_view name)
        {
            return ResourceID(std::string(kRuntimePrefix) + std::string(name));
        }

        static ResourceID MakeBuiltin(std::string_view name)
        {
            return ResourceID(std::string(kBuiltinPrefix) + std::string(name));
        }

        ResourceID MakeSibling(std::string_view relativeName) const
        {
            Assert(GetType() == ResourceIDType::Path);

            std::filesystem::path fsPath(GetValue());
            auto siblingPath = fsPath.parent_path() / relativeName;

            return ResourceID::MakePath(siblingPath.string());
        }

        ResourceIDType GetType() const
        {
            if (m_value.empty()) return ResourceIDType::Invalid;
            if (m_value.starts_with(kBuiltinPrefix)) return ResourceIDType::Builtin;
            if (m_value.starts_with(kRuntimePrefix)) return ResourceIDType::Runtime;
            if (m_value.starts_with(kPathPrefix)) return ResourceIDType::Path;

            return ResourceIDType::Invalid;
        }

        std::string_view GetValue() const
        {
            switch (GetType())
            {
            case ResourceIDType::Builtin: return std::string_view(m_value).substr(kBuiltinPrefix.size());
            case ResourceIDType::Runtime: return std::string_view(m_value).substr(kRuntimePrefix.size());
            case ResourceIDType::Path:    return std::string_view(m_value).substr(kPathPrefix.size());
            default:
                return {};
            }
        }

        bool IsValid() const
        {
            return GetType() != ResourceIDType::Invalid;
        }

        const std::string& String() const { return m_value; }
        std::string& String() { return m_value; }
        const char* c_str() const { return m_value.c_str(); }
        bool empty() const { return m_value.empty(); }
        void clear() { m_value.clear(); }

    private:
        explicit ResourceID(const char* str) : m_value(str) {}
        explicit ResourceID(std::string str) : m_value(std::move(str)) {}

        std::string m_value;
    };
}

// std::unordered_map / std::unordered_set을 위해 std::hash 특수화 export
namespace std
{
    template<>
    struct hash<Core::ResourceID>
    {
        std::size_t operator()(const Core::ResourceID& id) const noexcept
        {
            return std::hash<std::string>{}(id.String());
        }
    };
}