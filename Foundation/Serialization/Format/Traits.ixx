module;

#include "nlohmann/json.hpp"

export module Serialization:Traits;

import std;
import Core.Utils;
import :TraitsHelper;

// 읽기는 타입이 맞지 않으면 nullopt이다 (예외를 던지지 않는다). 숫자는 담을 수 없는 범위도 nullopt이다.
template<typename T>
std::optional<T> ReadNumber(const nlohmann::json& dataJ)
{
	if constexpr (std::is_integral_v<T>)
	{
		if (dataJ.is_number_unsigned())
		{
			const auto value = dataJ.get<std::uint64_t>();
			return std::in_range<T>(value) ? std::optional<T>{ static_cast<T>(value) } : std::nullopt;
		}
		if (dataJ.is_number_integer())
		{
			const auto value = dataJ.get<std::int64_t>();
			return std::in_range<T>(value) ? std::optional<T>{ static_cast<T>(value) } : std::nullopt;
		}
		return std::nullopt;   // 실수나 문자열은 정수가 아니다
	}
	else
	{
		static_assert(std::is_floating_point_v<T>);
		if (!dataJ.is_number())
			return std::nullopt;

		const double value = dataJ.get<double>();
		if constexpr (std::is_same_v<T, float>)
		{
			if (std::isfinite(value) && std::fabs(value) > std::numeric_limits<float>::max())
				return std::nullopt;   // float로 줄이면 무한대가 된다
		}
		return static_cast<T>(value);
	}
}

export template<typename T>
struct JsonTraitsBase
{
	static nlohmann::json SerializeToJson(const T& data) { return data; }

	static std::optional<T> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if constexpr (std::is_same_v<T, bool>)
		{
			if (!dataJ.is_boolean())
				return std::nullopt;
			return dataJ.get<bool>();
		}
		else
		{
			static_assert(std::is_arithmetic_v<T>, "이 타입의 JsonTraits 특수화가 필요하다");
			return ReadNumber<T>(dataJ);
		}
	}
};

template<>
struct JsonTraitsBase<std::string>
{
	static nlohmann::json SerializeToJson(const std::string& data) { return data; }
	static std::optional<std::string> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return std::nullopt;
		return dataJ.get<std::string>();
	}
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
	static std::optional<float> DeserializeFromJson(const nlohmann::json& dataJ) { return ReadNumber<float>(dataJ); }
};

template<>
struct JsonTraitsBase<std::wstring>
{
	static nlohmann::json SerializeToJson(const std::wstring& data) { return Core::WStringToString(data); }
	static std::optional<std::wstring> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return std::nullopt;
		return Core::StringToWString(dataJ.get<std::string>());
	}
};

// Count가 있는 enum은 이름(문자열)으로 저장한다. enum마다 Core::EnumToStringMap<T>() 특수화(이름 표) 한 줄만 있으면 된다.
// 파일에 없거나 모르는 이름이면 Core::InvalidEnum<T>를 돌려준다. 읽은 쪽에서 `== Core::InvalidEnum<T>`로 걸러낸다.
template<Core::CountEnum T>
struct JsonTraitsBase<T>
{
	static nlohmann::json SerializeToJson(const T& data) { return Core::EnumToString(data); }
	static std::optional<T> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return Core::InvalidEnum<T>;
		return Core::StringToEnum<T>(dataJ.get<std::string>()).value_or(Core::InvalidEnum<T>);
	}
};

export template<typename T>
struct JsonTraits : public JsonTraitsBase<T> {};
