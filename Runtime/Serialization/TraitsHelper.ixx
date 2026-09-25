module;

#include <nlohmann/json.hpp>

export module Runtime.Serialization:TraitsHelper;

import std;
import Core.Assert;

template<typename T, typename J>
void SafeRead(T& out, const J& value)
{
	Core::Assert(!value.is_null());
	out = value.template get<T>();
}

// x, y 공통 직렬화 헬퍼
export template<typename T>
nlohmann::json SerializeXY(const T& data)
{
	nlohmann::json j;
	j["x"] = data.x;
	j["y"] = data.y;
	return j;
}

// x, y 공통 역직렬화 헬퍼
export template<typename T>
void DeserializeXY(T& data, const nlohmann::json& j)
{
	SafeRead(data.x, j["x"]);
	SafeRead(data.y, j["y"]);
}

// 데이터 생성 및 채우기 범용 헬퍼
export template<typename T, typename Func>
T CreateAndFill(Func&& func)
{
	T data{};
	std::forward<Func>(func)(data);
	return data;
}