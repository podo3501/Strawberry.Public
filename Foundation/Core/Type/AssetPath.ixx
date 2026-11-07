export module Core.AssetPath;

import std;
import Core.Assert;
import Core.Utils;

// 에셋 저장소 루트 아래의 상대 경로. IAssetStorage가 파일을 가리킬 때 쓴다.
// - 구분자는 '/'로 통일하고 lexically_normal로 정리한다 ("a/./b", "a//b" -> "a/b"). 대소문자는 보존한다.
// - 비교와 해시는 대소문자를 무시한다 (Windows에서 같은 파일이 다른 키가 되지 않게). 문자열 자체는 바뀌지 않는다.
// - 절대 경로(드라이브 문자, 앞의 '/')와 저장소 밖으로 나가는 '..'는 거부한다. 확장자는 요구하지 않는다 (폴더나 확장자 없는 파일도 가리킬 수 있다).
// - 문자열에서 암묵 변환되지 않는다. 검증이 필요한 곳에서 Make/TryMake로 명시적으로 만든다.
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

            const std::filesystem::path normalized = std::filesystem::path(path).lexically_normal();
            if (normalized.empty() || normalized.has_root_name() || normalized.has_root_directory())
                return std::nullopt;

            for (const auto& element : normalized)
                if (element == "..")
                    return std::nullopt;   // lexically_normal이 정리하고도 남은 '..'은 저장소 밖으로 나간다

            const std::string text = normalized.generic_string();
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
            return TryMake((std::filesystem::path(m_value).parent_path() / relativeName).generic_string());
        }

        bool IsValid() const { return !m_value.empty(); }
        bool HasExtension() const { return ToPath().has_extension(); }   // 마지막 이름에 확장자가 있는가 ("a/b.json" true, "a/b"와 "a/b/" false)
        const std::string& String() const { return m_value; }   // '/' 구분, 대소문자 보존
        std::filesystem::path ToPath() const { return std::filesystem::path(m_value); }

        // 비교와 해시는 대소문자를 무시한다
        bool operator==(const AssetPath& other) const { return m_key == other.m_key; }
        auto operator<=>(const AssetPath& other) const { return m_key <=> other.m_key; }
        std::size_t GetHash() const noexcept { return std::hash<std::string>{}(m_key); }

    private:
        explicit AssetPath(std::string value)
            : m_value(std::move(value)), m_key(ToLowerCopy(m_value)) {}

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
