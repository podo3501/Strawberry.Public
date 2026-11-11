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

        // Path 종류의 ID는 대소문자를 무시하고 비교한다 ("Assets/Tex.png" == "assets/tex.png"). 값은 만든 대로 보존된다.
        // Runtime/Builtin은 이름을 그대로 비교한다 (대소문자 구분)
        bool operator==(const ResourceID& other) const { return m_key == other.m_key; }
        auto operator<=>(const ResourceID& other) const { return m_key <=> other.m_key; }

        // 코드에 적힌 상수용이다. 경로 규칙(AssetPath)이나 확장자에 어긋나면 Assert다. 파일에서 읽은 문자열은 TryMakePath를 쓴다
        static ResourceID MakePath(std::string_view path)
        {
            return MakePath(AssetPath::Make(path));
        }

        // 규칙에 어긋나거나 확장자가 없으면 nullopt이다. 파일에서 읽은 문자열용이다
        static std::optional<ResourceID> TryMakePath(std::string_view path)
        {
            const auto assetPath = AssetPath::TryMake(path);
            if (!assetPath || !assetPath->HasExtension())
                return std::nullopt;

            return MakePath(*assetPath);
        }

        // AssetPath로 만든다. 에셋 식별자는 파일이므로 확장자가 있어야 한다 (AssetPath 자체는 확장자를 요구하지 않는다)
        static ResourceID MakePath(const AssetPath& path)
        {
            Assert(path.HasExtension());
            return ResourceID(ResourceIDType::Path, std::string(kPathPrefix) + path.String());
        }

        // Path 종류의 ID가 가리키는 파일 경로 (만든 대소문자 그대로). Path 종류가 아니거나 AssetPath 규칙에 어긋나면 nullopt
        std::optional<AssetPath> GetPath() const
        {
            if (GetType() != ResourceIDType::Path)
                return std::nullopt;
            return AssetPath::TryMake(GetValue());
        }

        static ResourceID MakeRuntime(std::string_view name)
        {
            return ResourceID(ResourceIDType::Runtime, std::string(kRuntimePrefix) + std::string(name));
        }

        static ResourceID MakeBuiltin(std::string_view name)
        {
            return ResourceID(ResourceIDType::Builtin, std::string(kBuiltinPrefix) + std::string(name));
        }

        // 같은 폴더의 다른 파일 ID (파일에서 읽은 이름용). Path 종류가 아니거나 이름이 규칙에 어긋나면 nullopt
        // (저장소 밖, 절대 경로, 확장자 없음, 올바르지 않은 UTF-8)
        std::optional<ResourceID> TryMakeSibling(std::string_view relativeName) const
        {
            const auto path = GetPath();
            if (!path)
                return std::nullopt;

            const auto sibling = path->Sibling(relativeName);
            if (!sibling || !sibling->HasExtension())
                return std::nullopt;

            return MakePath(*sibling);
        }

        std::size_t GetHash() const noexcept { return std::hash<std::string>{}(m_key); }

        ResourceIDType GetType() const { return m_type; }

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

        bool IsValid() const { return m_type != ResourceIDType::Invalid; }

        const std::string& String() const { return m_value; }
        const char* c_str() const { return m_value.c_str(); }

    private:
        ResourceID(ResourceIDType type, std::string prefixedValue)
            : m_value(std::move(prefixedValue))
            , m_key(type == ResourceIDType::Path ? ToLower(m_value) : m_value)
            , m_type(type) {}

        std::string m_value;
        std::string m_key;   // 비교와 해시용. Path 종류만 소문자로 접는다
        ResourceIDType m_type{ ResourceIDType::Invalid };   // 만들 때 정한다 (이름이 접두사 모양이어도 바뀌지 않는다)
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