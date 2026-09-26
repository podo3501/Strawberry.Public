module;

#include <nlohmann/json.hpp>

export module Runtime.Serialization:ClassSerialize;

import std;
import :Concepts;
import :KeyConverter;
import :Traits;
import Core.Utils;

//새로운 Serialize를 사용하는 클래스를 만들면 여기서 Extension을 만들어서 사용한다.
//SerializeClass 및 DeserializeClass 함수를 각 클래스에 맞게 정의해서 파일을 만든다.

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

export template<typename T>
void DeserializeClass_Internal(const nlohmann::json& outReadJ, T& data)
{
	Serializer serializer{ outReadJ };
	if constexpr (RawPointerLike<T>)
		data->Serialize(serializer);
	else
		data.Serialize(serializer);
}

export template<typename T>
void DeserializeClass(const nlohmann::json& j, T& data)
{ 
	DeserializeClass_Internal(j, data); 
}

export template<typename T>
void DeserializeClass(const nlohmann::json& j, std::unique_ptr<T>& data)
{
	data = std::make_unique<T>();
	DeserializeClass_Internal(j, *data);
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

export template<typename T>
	T DeserializeByType(const nlohmann::json& v)
{
	if constexpr (HasSerialize<T>)
	{
		T data{};
		DeserializeClass(v, data);
		return data;
	}
	else
	{
		return JsonTraits<T>::DeserializeFromJson(v);
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
	void DeserializeMapContainer(const nlohmann::json& j, MapContainer& datas)
{
	using K = typename MapContainer::key_type;
	using T = typename MapContainer::mapped_type;

	for (const auto& [k, v] : j.items())
	{
		const auto key = FromKeyString<K>(k);
		datas.emplace(key, DeserializeByType<T>(v));
	}
}

export template<typename SeqContainer> // Sequence 컨테이너 전용 직렬화 / 역직렬화
void SerializeSeqContainer(nlohmann::json& j, SeqContainer& datas)
{
	for (auto& data : datas)
		j.push_back(SerializeByType(data));
}

export template<typename SeqContainer>
	void DeserializeSeqContainer(const nlohmann::json& j, SeqContainer& datas)
{
	for (const auto& dataJ : j)
	{
		using T = typename SeqContainer::value_type;
		datas.emplace_back(DeserializeByType<T>(dataJ));
	}
}