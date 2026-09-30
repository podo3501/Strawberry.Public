export module Runtime.Render.Command:Type;

import std;

export enum class CommandType
{
	None,
	Direct,  // 렌더링
	Copy,    // 리소스 전송
	Compute  // 계산
};

export using FenceID = std::uint64_t;
export inline constexpr FenceID InvalidFenceID = 0; // fence 값 0은 실제 값으로 쓰지 않는다.