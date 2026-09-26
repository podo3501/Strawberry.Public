export module Runtime.Serialization:Concepts;

import std;
import Core.Utils;

export class Serializer;
export template<typename T> // SerializeIO 지원 여부 확인 컨셉
concept HasSerialize =
	requires(T t, Serializer & serializer) {
		{ t->Serialize(serializer) };   // 스마트 포인터용
} ||
	requires(T t, Serializer & serializer) {
		{ t.Serialize(serializer) };    // 일반 객체용
};

export template<typename T> //멤버 begin/end가 있는 경우
concept HasMemberBeginEnd = requires(T t) {
	{ t.begin() } -> std::input_iterator;
	{ t.end() } -> std::sentinel_for<decltype(t.begin())>;
};

export template<typename T> // 기본 문자열 계열 및 filesystem::path 제외 컨셉
concept NotBasicString =
!(std::same_as<std::remove_cvref_t<T>, std::string> ||
	std::same_as<std::remove_cvref_t<T>, std::wstring> ||
	std::same_as<std::remove_cvref_t<T>, std::u16string> ||
	std::same_as<std::remove_cvref_t<T>, std::u32string> ||
	std::same_as<std::remove_cvref_t<T>, std::filesystem::path>);

export template<typename T> // Sequence 컨테이너 검사 컨셉
concept SequenceLike =
!requires { typename T::key_type; }&&     // key_type 없어야 함 (map 방지)
	requires { typename T::value_type; }&&    // value_type 있어야 함 (sequence 조건)
	HasMemberBeginEnd<T>&&
	NotBasicString<T>;                          // 문자열 계열 제외

export template<typename T> // Map 계열 컨테이너 검사 컨셉
concept MapLike = requires {
	typename T::key_type;
	typename T::mapped_type; }&&
	HasMemberBeginEnd<T>;