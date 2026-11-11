export module Core.AssetPath;

import std;
import Core.Assert;
import Core.Utils;

// 에셋 저장소 루트 아래의 상대 경로. IAssetStorage가 파일을 가리킬 때 쓴다 (규칙은 path-usage.md).
// 구분자는 '/'로 통일하고 대소문자는 보존하며, 비교와 해시는 대소문자를 무시한다.
// 절대 경로와 저장소 밖으로 나가는 '..'는 거부하고, 확장자는 요구하지 않는다.
// 문자열에서 암묵 변환되지 않는다. Make/TryMake로 명시적으로 만든다.
namespace Core
{
    export class AssetPath
    {
    public:
        AssetPath() = default;   // 빈 경로 (IsValid()가 false). Make/TryMake로만 유효한 경로가 만들어진다

        // 규칙에 어긋나면 nullopt
        static std::optional<AssetPath> TryMake(std::string_view path)
        {
            if (path.empty() || path.find('\0') != std::string_view::npos)
                return std::nullopt;

            if (!IsValidUtf8(path))
                return std::nullopt;

            const std::filesystem::path normalized = PathFromUtf8(path).lexically_normal();
            if (normalized.empty() || normalized.has_root_name() || normalized.has_root_directory())
                return std::nullopt;

            for (const auto& element : normalized)
                if (element == "..")
                    return std::nullopt;   // lexically_normal이 정리하고도 남은 '..'은 저장소 밖으로 나간다

            const std::string text = PathToUtf8(normalized);
            if (text == ".")
                return std::nullopt;   // 저장소 루트 자신은 가리키지 않는다

            return AssetPath(text);
        }

        // 규칙에 어긋나면 Assert. 코드에 적힌 상수처럼 항상 유효해야 하는 경로에 쓴다
        static AssetPath Make(std::string_view path)
        {
            auto made = TryMake(path);
            Assert(made.has_value() && "AssetPath: 상대 경로가 아니거나 '..'으로 저장소 밖을 가리킴");
            return made.value_or(AssetPath{});
        }

        // 같은 폴더의 다른 파일 ("a/b.json" + "b.meta" -> "a/b.meta"). 결과가 규칙에 어긋나면 nullopt
        std::optional<AssetPath> Sibling(std::string_view relativeName) const
        {
            if (!IsValidUtf8(relativeName))
                return std::nullopt;

            return TryMake(PathToUtf8(PathFromUtf8(m_value).parent_path() / PathFromUtf8(relativeName)));
        }

        bool IsValid() const { return !m_value.empty(); }
        bool HasExtension() const { return ToPath().has_extension(); }   // 마지막 이름에 확장자가 있는가 ("a/b.json" true, "a/b"와 "a/b/" false)
        std::string LowerExtension() const { return ToLower(PathToUtf8(ToPath().extension())); }   // 마지막 이름의 확장자를 소문자로 (점 포함, 없으면 빈 문자열). 로더를 찾는 키다
        const std::string& String() const { return m_value; }   // '/' 구분, 대소문자 보존
        std::filesystem::path ToPath() const { return PathFromUtf8(m_value); }

        // 비교와 해시는 대소문자를 무시한다
        bool operator==(const AssetPath& other) const { return m_key == other.m_key; }
        auto operator<=>(const AssetPath& other) const { return m_key <=> other.m_key; }
        std::size_t GetHash() const noexcept { return std::hash<std::string>{}(m_key); }

    private:
        explicit AssetPath(std::string value)
            : m_value(std::move(value)), m_key(ToLower(m_value)) {}

        std::string m_value;
        std::string m_key;   // 비교용 소문자
    };
}

namespace std
{
    template<>
    struct hash<Core::AssetPath>
    {
        std::size_t operator()(const Core::AssetPath& path) const noexcept { return path.GetHash(); }
    };
}
