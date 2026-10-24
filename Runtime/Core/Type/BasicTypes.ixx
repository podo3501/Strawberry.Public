export module Core.BasicTypes;

import std;

export namespace Core
{
    // 정수 및 바이트 타입 정의
	using Index = std::uint32_t;
	constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

	using Byte = std::byte;
	using ByteBuffer = std::vector<Byte>;
}