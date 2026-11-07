export module Core.ResourceID;

import std;
import Core.AssetPath;
import Core.Assert;
import Core.Utils;

namespace Core
{
    inline constexpr std::string_view kBuiltinPrefix = "builtin://";
    inline constexpr std::string_view kRuntimePrefix = "runtime://";
    inline constexpr std::string_view kPathPrefix = "path://";

    // 경로 정규화: 구분자를 '/'로 통일하고 "a/./b", "a//b"를 정리한다. 대소문자는 보존한다 (같은 ID인지의 비교는 ResourceID가 대소문자를 무시하고 한다)
    std::string NormalizePath(const std::filesystem::path& path)
    {
        return path.lexically_normal().generic_string();
    }

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

        // Path 종류의 ID는 대소문자를 무시하고 비교한다 ("Assets/Tex.png"와 "assets/tex.png"는 같은 ID). 값(String, GetValue)은 만든 대로 보존된다.
        // Runtime/Builtin은 이름을 그대로 비교한다 (대소문자 구분)
        bool operator==(const ResourceID& other) const { return m_key == other.m_key; }
        auto operator<=>(const ResourceID& other) const { return m_key <=> other.m_key; }

        static ResourceID MakePath(std::string_view path)
        {
            std::filesystem::path fsPath(path);

            Assert(fsPath.has_extension());
            return ResourceID(std::string(kPathPrefix) + NormalizePath(fsPath));
        }

        // AssetPath로 만든다. 에셋 식별자는 파일이므로 확장자가 있어야 한다 (AssetPath 자체는 확장자를 요구하지 않는다)
        static ResourceID MakePath(const AssetPath& path)
        {
            return MakePath(path.String());
        }

        // Path 종류의 ID가 가리키는 파일 경로 (만든 대소문자 그대로). Path 종류가 아니거나 AssetPath 규칙(상대 경로, 저장소 밖으로 나가는 '..' 금지)에 어긋나면 nullopt
        std::optional<AssetPath> GetPath() const
        {
            if (GetType() != ResourceIDType::Path)
                return std::nullopt;
            return AssetPath::TryMake(GetValue());
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

        std::size_t GetHash() const noexcept { return std::hash<std::string>{}(m_key); }

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
        const char* c_str() const { return m_value.c_str(); }
        bool empty() const { return m_value.empty(); }

    private:
        explicit ResourceID(std::string str)
            : m_value(std::move(str))
            , m_key(m_value.starts_with(kPathPrefix) ? ToLowerCopy(m_value) : m_value) {}

        std::string m_value;
        std::string m_key;   // 비교와 해시용. Path 종류만 소문자로 접는다
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
            return id.GetHash();
        }
    };
}