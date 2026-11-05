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

// filesystem::path는 utf-8로 저장하고 읽어들인다.
template<>
struct JsonTraitsBase<std::filesystem::path>
{
	static nlohmann::json SerializeToJson(const std::filesystem::path& data) { return data.string(); }
	static std::filesystem::path DeserializeFromJson(const nlohmann::json& dataJ)
	{
		return std::filesystem::path(dataJ.get<std::string>());
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