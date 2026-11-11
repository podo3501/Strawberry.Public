module;

#include <nlohmann/json.hpp>

export module Serialization:ClassSerialize;

import std;
import :Concepts;
import :KeyConverter;
import :Traits;
import Core.Utils;

//새로운 Serialize를 사용하는 클래스를 만들면 여기서 Extension을 만들어서 사용한다.
//SerializeClass 및 DeserializeClass 함수를 각 클래스에 맞게 정의해서 파일을 만든다.

// 읽다가 생긴 오류를 읽고 있는 Serializer에 알린다 (parent가 없으면 알릴 곳이 없다). Serializer는 아래에서 정의된다
template<typename S>
void ReportErrorOn(S* serializer, std::string_view message) noexcept
{
	if (serializer)
		serializer->ReportError(message);
}

export template<typename T>
inline void SerializeClass_GenerateJson(nlohmann::json& outData, T& data)
{
	Serializer serializer{ outData };
	if constexpr (RawPointerLike<T>)
		data->Serialize(serializer);
	else
		data.Serialize(serializer);
}

export template<typename T>
void SerializeClass_Internal(nlohmann::json& j, T& data) { SerializeClass_GenerateJson(j, data); }

export template<typename T>
void SerializeClass(nlohmann::json& j, T& data) { SerializeClass_Internal(j, data); }
export template<typename T>
void SerializeClass(nlohmann::json& j, std::unique_ptr<T>& data) { SerializeClass_Internal(j, *data); }

///////////////////////////////////////////////////////

// parent는 이 클래스를 포함하는 쪽의 Serializer다. 오류가 거기까지 전해진다
export template<typename T>
void DeserializeClass_Internal(const nlohmann::json& outReadJ, T& data, Serializer* parent)
{
	Serializer serializer{ outReadJ, parent };
	if constexpr (RawPointerLike<T>)
		data->Serialize(serializer);
	else
		data.Serialize(serializer);
}

export template<typename T>
void DeserializeClass(const nlohmann::json& j, T& data, Serializer* parent = nullptr)
{
	DeserializeClass_Internal(j, data, parent);
}

export template<typename T>
void DeserializeClass(const nlohmann::json& j, std::unique_ptr<T>& data, Serializer* parent = nullptr)
{
	data = std::make_unique<T>();
	DeserializeClass_Internal(j, *data, parent);
}

///////////////////////////////////////////////////////

export template<typename T> // 타입별 자동 분기 직렬화/역직렬화
nlohmann::json SerializeByType(T& v)
{
	if constexpr (HasSerialize<T>)
	{
		nlohmann::json j{};
		SerializeClass(j, v);
		return j;
	}
	else
	{
		return JsonTraits<T>::SerializeToJson(v);
	}
}

// 읽을 수 없으면 오류를 알리고 nullopt이다
export template<typename T>
std::optional<T> DeserializeByType(const nlohmann::json& v, Serializer* parent)
{
	if constexpr (HasSerialize<T>)
	{
		// 쓸 때 키가 하나도 없는 클래스는 null로 적히므로 빈 객체로 읽는다
		const nlohmann::json empty = nlohmann::json::object();
		if (!v.is_object() && !v.is_null())
		{
			ReportErrorOn(parent, "객체가 아닌 값");
			return std::nullopt;
		}

		T data{};
		DeserializeClass(v.is_null() ? empty : v, data, parent);
		return data;
	}
	else
	{
		auto data = JsonTraits<T>::DeserializeFromJson(v);
		if (!data)
			ReportErrorOn(parent, "값의 형식이 틀림");
		return data;
	}
}

export template<typename MapContainer> // Map 컨테이너 전용 직렬화 / 역직렬화
void SerializeMapContainer(nlohmann::json& j, MapContainer& datas)
{
	using T = typename MapContainer::mapped_type;

	for (auto& [k, v] : datas)
	{
		const auto key = ToKeyString(k);
		j.emplace(key, SerializeByType<T>(v));
	}
}

export template<typename MapContainer>
	void DeserializeMapContainer(const nlohmann::json& j, MapContainer& datas, Serializer* parent)
{
	using K = typename MapContainer::key_type;
	using T = typename MapContainer::mapped_type;

	if (j.is_null())
		return;   // 항목이 하나도 없는 표는 쓸 때 null로 적힌다
	if (!j.is_object())
	{
		ReportErrorOn(parent, "객체가 아닌 값 (이름 표)");
		return;
	}

	for (const auto& [k, v] : j.items())
	{
		auto key = FromKeyString<K>(k);
		if (!key)
		{
			ReportErrorOn(parent, "이름 표의 키를 바꿀 수 없음");
			continue;
		}

		auto value = DeserializeByType<T>(v, parent);
		if (value)
			datas.emplace(std::move(*key), std::move(*value));
	}
}

export template<typename SeqContainer> // Sequence 컨테이너 전용 직렬화 / 역직렬화
void SerializeSeqContainer(nlohmann::json& j, SeqContainer& datas)
{
	for (auto& data : datas)
		j.push_back(SerializeByType(data));
}

export template<typename SeqContainer>
	void DeserializeSeqContainer(const nlohmann::json& j, SeqContainer& datas, Serializer* parent)
{
	using T = typename SeqContainer::value_type;

	if (j.is_null())
		return;   // 항목이 하나도 없는 목록은 쓸 때 null로 적힌다
	if (!j.is_array())
	{
		ReportErrorOn(parent, "배열이 아닌 값");
		return;
	}

	for (const auto& dataJ : j)
	{
		auto value = DeserializeByType<T>(dataJ, parent);
		if (value)
			datas.emplace_back(std::move(*value));
	}
}
