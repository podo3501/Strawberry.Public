module;

#include "nlohmann/json.hpp"

export module Runtime.Serialization:Traits;

import std;
import Core.Utils;
import :TraitsHelper;

export template<typename T>
struct JsonTraitsBase
{
	static nlohmann::json SerializeToJson(const T& data) { return data; }
	static T DeserializeFromJson(const nlohmann::json& dataJ) { return dataJ; }
};

export template<>
struct JsonTraitsBase<std::string>
{
	static nlohmann::json SerializeToJson(const std::string& data) { return data; }
	static std::string DeserializeFromJson(const nlohmann::json& dataJ) { return dataJ; }
};

export template<>
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
export template<>
struct JsonTraitsBase<std::filesystem::path>
{
	static nlohmann::json SerializeToJson(const std::filesystem::path& data) { return data.string(); }
	static std::filesystem::path DeserializeFromJson(const nlohmann::json& dataJ)
	{
		return std::filesystem::path(dataJ.get<std::string>());
	}
};

export template<typename T>
struct JsonTraits : public JsonTraitsBase<T> {};