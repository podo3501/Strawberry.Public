module;

#include <nlohmann/json.hpp>

export module Serialization:ValueIO;

import std;
import :Concepts;
import :KeyConverter;
import :Traits;
import :Serializer;
import Core.Utils;

// 값 하나(클래스, unique_ptr, 목록, 표, 그 밖의 값)를 JSON으로 쓴다. owner는 이 값을 포함하는 쪽의 Serializer다.
// 쓸 수 없으면(빈 포인터, 쓸 수 없는 값) 오류를 알리고 false이며, 항목이 없는 목록과 표는 out이 null로 남는다.
template<typename T>
bool WriteValue(nlohmann::json& out, T& value, Serializer& owner)
{
	if constexpr (OwnedSerializable<T>)
	{
		if (!value)
		{
			owner.ReportError("빈 포인터는 쓸 수 없음");
			return false;
		}
		return WriteValue(out, *value, owner);
	}
	else if constexpr (SerializableClass<T>)
	{
		auto writer = Serializer::ForWrite(out, &owner);
		value.Serialize(writer);   // 안에서 생긴 오류는 owner까지 전해진다
		return true;
	}
	else if constexpr (SequenceLike<T>)
	{
		std::size_t index = 0;
		for (auto& element : value)
		{
			Serializer::ItemScope scope{ owner, std::to_string(index++) };
			nlohmann::json item;
			if (WriteValue(item, element, owner))
				out.push_back(std::move(item));
		}
		return true;
	}
	else if constexpr (MapLike<T>)
	{
		// 올바르지 않은 UTF-8인 키와 이미 쓴 키와 겹치는 키(짝 없는 서로게이트가 모두 U+FFFD가 되는 wstring 키 등)는 그 항목을 건너뛴다
		for (auto& [key, element] : value)
		{
			const auto keyString = ToKeyString(key);
			if (!Core::IsValidUtf8(keyString))
			{
				owner.ReportError("표의 키가 올바른 UTF-8이 아님");
				continue;
			}
			if (out.contains(keyString))
			{
				owner.ReportError("표의 키가 겹침");
				continue;
			}

			Serializer::ItemScope scope{ owner, keyString };
			nlohmann::json item;
			if (WriteValue(item, element, owner))
				out.emplace(keyString, std::move(item));
		}
		return true;
	}
	else
	{
		static_assert(!HasMemberBeginEnd<T> || IsBasicString<T>, "지원하지 않는 컨테이너다 (목록은 emplace_back, 표는 key_type과 mapped_type이 필요하다)");
		static_assert(!std::is_pointer_v<T> && !requires { typename T::element_type; }, "포인터는 Serialize를 가진 클래스의 unique_ptr만 쓸 수 있다");

		auto json = JsonTraits<T>::SerializeToJson(value);
		if (!json)
		{
			owner.ReportError("값을 쓸 수 없음");
			return false;
		}
		out = std::move(*json);
		return true;
	}
}

// JSON에서 값 하나를 제자리에 읽는다. 읽을 수 없으면 오류를 알리고 false이며 value는 그대로다.
// 목록과 표는 읽을 때 비우고 채운다 (null은 빈 값). 틀린 항목은 오류를 알리고 건너뛴다.
template<typename T>
bool ReadValue(const nlohmann::json& in, T& value, Serializer& owner)
{
	if constexpr (OwnedSerializable<T>)
	{
		auto created = std::make_unique<typename T::element_type>();
		if (!ReadValue(in, *created, owner))
			return false;
		value = std::move(created);
		return true;
	}
	else if constexpr (SerializableClass<T>)
	{
		if (!in.is_object() && !in.is_null())
		{
			owner.ReportError("객체가 아닌 값");
			return false;
		}

		const nlohmann::json empty = nlohmann::json::object();   // 쓸 때 키가 하나도 없는 클래스는 null로 적히므로 빈 객체로 읽는다
		auto reader = Serializer::ForRead(in.is_null() ? empty : in, &owner);
		value.Serialize(reader);
		return true;
	}
	else if constexpr (SequenceLike<T>)
	{
		static_assert(requires(T& list, typename T::value_type item) { list.emplace_back(std::move(item)); }, "목록을 읽으려면 emplace_back이 되는 컨테이너여야 한다 (std::array는 읽을 수 없다)");

		if (!in.is_array() && !in.is_null())
		{
			owner.ReportError("배열이 아닌 값");
			return false;
		}

		value.clear();   // 항목이 하나도 없는 목록은 쓸 때 null로 적힌다
		std::size_t index = 0;
		for (const auto& itemJson : in)
		{
			Serializer::ItemScope scope{ owner, std::to_string(index++) };
			typename T::value_type item{};
			if (ReadValue(itemJson, item, owner))
				value.emplace_back(std::move(item));
		}
		return true;
	}
	else if constexpr (MapLike<T>)
	{
		if (!in.is_object() && !in.is_null())
		{
			owner.ReportError("객체가 아닌 값 (이름 표)");
			return false;
		}

		value.clear();   // 항목이 하나도 없는 표는 쓸 때 null로 적힌다
		for (const auto& [keyString, itemJson] : in.items())
		{
			Serializer::ItemScope scope{ owner, keyString };
			auto key = FromKeyString<typename T::key_type>(keyString);
			if (!key)
			{
				owner.ReportError("이름 표의 키를 바꿀 수 없음");
				continue;
			}

			typename T::mapped_type item{};
			if (!ReadValue(itemJson, item, owner))
				continue;

			// 서로 다른 JSON 키가 같은 키가 되면(예: int 키 "7"과 "007") emplace가 실패한다
			if (!value.emplace(std::move(*key), std::move(item)).second)
				owner.ReportError("표의 키가 겹침");
		}
		return true;
	}
	else
	{
		static_assert(!HasMemberBeginEnd<T> || IsBasicString<T>, "지원하지 않는 컨테이너다 (목록은 emplace_back, 표는 key_type과 mapped_type이 필요하다)");
		static_assert(!std::is_pointer_v<T> && !requires { typename T::element_type; }, "포인터는 Serialize를 가진 클래스의 unique_ptr만 쓸 수 있다");

		auto parsed = JsonTraits<T>::DeserializeFromJson(in);
		if (!parsed)
		{
			owner.ReportError("값의 형식이 틀림");
			return false;
		}
		value = std::move(*parsed);
		return true;
	}
}

template<typename T>
void Serializer::Process(const std::string& key, T& data) noexcept
{
	m_key = key;
	if (IsWrite())
	{
		if (m_write->is_object() || m_write->is_null())   // 아니면 만들 때 이미 오류로 알렸다
		{
			nlohmann::json value;
			WriteValue(value, data, *this);   // 쓸 수 없으면 오류가 기록되고 키는 null로 남는다
			(*m_write)[key] = std::move(value);
		}
	}
	else if (const auto found = m_read->find(key); found != m_read->end())
		ReadValue(*found, data, *this);       // 읽을 수 없으면 오류가 기록되고 기존 값이 남는다
}
