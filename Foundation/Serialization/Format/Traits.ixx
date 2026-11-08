module;

#include "nlohmann/json.hpp"

export module Serialization:Traits;

import std;
import Core.Utils;
import :TraitsHelper;

export template<typename T>
struct JsonTraitsBase
{
	static nlohmann::json SerializeToJson(const T& data) { return data; }
	static T DeserializeFromJson(const nlohmann::json& dataJ) { return dataJ; }
};

template<>
struct JsonTraitsBase<std::string>
{
	static nlohmann::json SerializeToJson(const std::string& data) { return data; }
	static std::string DeserializeFromJson(const nlohmann::json& dataJ) { return dataJ; }
};

// float를 double로 그대로 넣으면 1.1f가 1.100000023841858로 적힌다. float를 되살릴 수 있는 가장 짧은 십진수(1.1)로 적는다.
// 읽을 때 double 1.1을 float로 줄이면 원래 float와 같은 값이 된다.
template<>
struct JsonTraitsBase<float>
{
	static nlohmann::json SerializeToJson(const float& data)
	{
		if (!std::isfinite(data)) return data;
		std::array<char, 32> buffer{};
		const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), data);
		if (result.ec != std::errc{}) return data;
		return std::strtod(std::string(buffer.data(), result.ptr).c_str(), nullptr);
	}
	static float DeserializeFromJson(const nlohmann::json& dataJ) { return dataJ; }
};

template<>
struct JsonTraitsBase<std::wstring>
{
	static nlohmann::json SerializeToJson(const std::wstring& data) { return Core::WStringToString(data); }
	static std::wstring DeserializeFromJson(const nlohmann::json& dataJ)
	{
		return CreateAndFill<std::wstring>([&dataJ](std::wstring& data) {
			data = Core::StringToWString(dataJ); });
	}
};

// Count가 있는 enum은 이름(문자열)으로 저장한다. enum마다 Core::EnumToStringMap<T>() 특수화(이름 표) 한 줄만 있으면 된다.
// 파일에 없거나 모르는 이름이면 Core::InvalidEnum<T>를 돌려준다. 읽은 쪽에서 `== Core::InvalidEnum<T>`로 걸러낸다.
template<Core::CountEnum T>
struct JsonTraitsBase<T>
{
	static nlohmann::json SerializeToJson(const T& data) { return Core::EnumToString(data); }
	static T DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return Core::InvalidEnum<T>;
		return Core::StringToEnum<T>(dataJ.get<std::string>()).value_or(Core::InvalidEnum<T>);
	}
};

export template<typename T>
struct JsonTraits : public JsonTraitsBase<T> {};