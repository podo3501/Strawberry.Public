export module Core.Utils:EnumUtils;

import std;
import Core.Logger;

namespace Core
{
    // 마지막 원소는 Count가 되어야 한다. InvalidEnum은 제일 큰 값으로 강제 설정한 값이라 이름 표 밖이다.
    export template<typename T>
    concept CountEnum =
        std::is_enum_v<T> &&
        requires { T::Count; };

    export template<CountEnum T>
    inline constexpr T InvalidEnum = static_cast<T>(std::numeric_limits<std::underlying_type_t<T>>::max());

    export template<CountEnum T>
    inline constexpr std::size_t EnumSize = static_cast<std::size_t>(T::Count); // 사용법 EnumSize<xxx> 뒤에 괄호가 없다.

    export template<typename T>
        requires std::is_enum_v<T>
    constexpr auto ToIndex(T enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(enumerator);
    }

    // 이름은 enum 선언 순서와 같아야 한다 (개수만 검사한다)
    export template<CountEnum T, typename... Names>
        requires (sizeof...(Names) == EnumSize<T>)
    constexpr auto MakeEnumStringMap(Names... names)
    {
        return std::array<const char*, EnumSize<T>>{ names... };
    }

    export template<CountEnum T>
    constexpr std::array<const char*, EnumSize<T>> EnumToStringMap();

    // InvalidEnum처럼 이름 표 밖의 값은 파일에서 올 수 있으므로 오류를 남기고 nullopt이다
    export template<CountEnum T>
    std::optional<std::string_view> EnumToString(T value)
    {
        constexpr auto list = EnumToStringMap<T>();
        const auto idx = static_cast<std::size_t>(ToIndex(value));
        if (idx >= list.size())
        {
            Logger::Error("EnumToString: 이름 표 밖의 값 {}", idx);
            return std::nullopt;
        }

        return std::string_view(list[idx]);
    }

    export template<CountEnum T>
    std::optional<T> StringToEnum(std::string_view str)
    {
        constexpr auto list = EnumToStringMap<T>();
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            if (str == list[i])
                return static_cast<T>(i);
        }
        return std::nullopt;
    }

    // for에서 사용하기 좋다. 0부터 시작한다.
    export template<CountEnum T>
    inline constexpr auto EnumValues = [] {
        std::array<T, EnumSize<T>> v{};
        for (std::size_t i = 0; i < v.size(); ++i) v[i] = static_cast<T>(i);
        return v;
        }();
}
