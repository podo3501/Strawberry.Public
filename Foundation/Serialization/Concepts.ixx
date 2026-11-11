export module Serialization:Concepts;

import std;

export class Serializer;

// Serialize(Serializer&)를 가진 클래스
template<typename T>
concept SerializableClass = requires(T& t, Serializer& serializer) { t.Serialize(serializer); };

template<typename T>
struct IsUniquePtr : std::false_type {};

template<typename T, typename Deleter>
struct IsUniquePtr<std::unique_ptr<T, Deleter>> : std::true_type {};

// 그런 클래스의 unique_ptr (읽을 때마다 새 객체를 만든다)
template<typename T>
concept OwnedSerializable = IsUniquePtr<T>::value && SerializableClass<typename T::element_type>;

// 클래스이거나 그런 클래스의 unique_ptr. 일반 포인터와 shared_ptr은 지원하지 않는다
template<typename T>
concept HasSerialize = SerializableClass<T> || OwnedSerializable<T>;

template<typename T> // 멤버 begin/end가 있는 경우
concept HasMemberBeginEnd = requires(T t) {
	{ t.begin() } -> std::input_iterator;
	{ t.end() } -> std::sentinel_for<decltype(t.begin())>;
};

template<typename T> // 기본 문자열 계열
concept IsBasicString =
	std::same_as<std::remove_cvref_t<T>, std::string> ||
	std::same_as<std::remove_cvref_t<T>, std::wstring> ||
	std::same_as<std::remove_cvref_t<T>, std::u16string> ||
	std::same_as<std::remove_cvref_t<T>, std::u32string>;

template<typename T> // Sequence 컨테이너 (key_type이 없고 value_type이 있으며 문자열이 아님)
concept SequenceLike =
	!requires { typename T::key_type; } &&
	requires { typename T::value_type; } &&
	HasMemberBeginEnd<T> &&
	!IsBasicString<T>;

template<typename T> // Map 계열 컨테이너
concept MapLike =
	requires {
		typename T::key_type;
		typename T::mapped_type;
	} &&
	HasMemberBeginEnd<T>;
