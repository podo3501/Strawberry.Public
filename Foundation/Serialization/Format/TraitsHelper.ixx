module;

#include <nlohmann/json.hpp>

export module Serialization:TraitsHelper;

import std;

// x, y 공통 직렬화 헬퍼
export template<typename T>
nlohmann::json SerializeXY(const T& data)
{
	nlohmann::json j;
	j["x"] = data.x;
	j["y"] = data.y;
	return j;
}

// 데이터 생성 및 채우기 범용 헬퍼
export template<typename T, typename Func>
T CreateAndFill(Func&& func)
{
	T data{};
	std::forward<Func>(func)(data);
	return data;
}
