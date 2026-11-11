module;

#include <nlohmann/json.hpp>

export module Serialization:Traits;

import std;
import Core.Utils;

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
			// FLT_MAX에 반 ULP를 더한 값 이상이면 float로 줄일 때 무한대가 된다. FLT_MAX를 가장 짧은 십진수로 쓴 3.4028235e+38은 FLT_MAX로 줄어드니 받아들인다
			constexpr double FloatOverflowLimit = 0x1.ffffffp127;
			if (std::isfinite(value) && std::fabs(value) >= FloatOverflowLimit)
				return std::nullopt;
		}
		return static_cast<T>(value);
	}
}

export template<typename T>
struct JsonTraits
{
	// NaN과 무한대는 JSON에 null로 적혀 다시 읽을 수 없으므로 쓸 수 없다 (nullopt)
	static std::optional<nlohmann::json> SerializeToJson(const T& data)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			if (!std::isfinite(data))
				return std::nullopt;
		}
		return nlohmann::json(data);
	}

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
struct JsonTraits<std::string>
{
	// 올바르지 않은 UTF-8은 JSON을 문자열로 만들 때(dump) 예외가 나므로 쓸 수 없다 (nullopt)
	static std::optional<nlohmann::json> SerializeToJson(const std::string& data)
	{
		if (!Core::IsValidUtf8(data))
			return std::nullopt;
		return nlohmann::json(data);
	}
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
struct JsonTraits<float>
{
	static std::optional<nlohmann::json> SerializeToJson(const float& data)
	{
		if (!std::isfinite(data)) return std::nullopt;   // NaN과 무한대는 쓸 수 없다
		std::array<char, 32> buffer{};
		const auto written = std::to_chars(buffer.data(), buffer.data() + buffer.size(), data);
		if (written.ec != std::errc{}) return nlohmann::json(data);

		double shortest = 0.0;   // 로케일과 무관하게 십진수를 double로 읽는다
		const auto parsed = std::from_chars(buffer.data(), written.ptr, shortest);
		if (parsed.ec != std::errc{}) return nlohmann::json(data);
		return nlohmann::json(shortest);
	}
	static std::optional<float> DeserializeFromJson(const nlohmann::json& dataJ) { return ReadNumber<float>(dataJ); }
};

template<>
struct JsonTraits<std::wstring>
{
	static std::optional<nlohmann::json> SerializeToJson(const std::wstring& data) { return nlohmann::json(Core::WStringToUTF8(data)); }
	static std::optional<std::wstring> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return std::nullopt;
		return Core::UTF8ToWString(dataJ.get<std::string>());
	}
};

// Count가 있는 enum은 이름(문자열)으로 저장한다. enum마다 Core::EnumToStringMap<T>() 특수화(이름 표) 한 줄만 있으면 된다.
// 문자열이 아니거나 모르는 이름이면 읽을 수 없고, 이름 표 밖의 값(InvalidEnum 등)은 쓸 수 없다 (둘 다 nullopt).
template<Core::CountEnum T>
struct JsonTraits<T>
{
	static std::optional<nlohmann::json> SerializeToJson(const T& data)
	{
		const auto name = Core::EnumToString(data);
		if (!name)
			return std::nullopt;
		return nlohmann::json(std::string(*name));
	}
	static std::optional<T> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return std::nullopt;
		return Core::StringToEnum<T>(dataJ.get<std::string>());
	}
};

