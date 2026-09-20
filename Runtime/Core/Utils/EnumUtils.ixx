export module Core.Utils:Enum; // 모듈 이름은 프로젝트 규칙에 맞춰 변경 가능합니다.

import std;

export namespace Core
{
	//마지막 원소는 Count가 되어야 함. Invaild는 제일 큰 값으로 강제 설정. 그래서 IsValid에서 false가 나오게 됨.
	template<typename T>
	concept CountEnum =
		std::is_enum_v<T> &&
		requires { T::Count; };

	template<CountEnum T>
	inline constexpr std::size_t EnumSize = static_cast<std::size_t>(T::Count); //사용법 EnumSize<xxx> 뒤에 괄호가 없다.

	template <CountEnum T>
	constexpr auto ToIndex(T enumerator) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(enumerator);
	}

	// 일반 Enum일 때 호출됨
	template<typename T>
		requires std::is_enum_v<T> && (!CountEnum<T>)
	constexpr auto ToIndex(T value) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}
}