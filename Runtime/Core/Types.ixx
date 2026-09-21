export module Core.Types;

import std;

namespace Core
{
    // 정수 및 바이트 타입 정의
    export using Index = std::uint32_t;
    export constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

    export using Byte = std::byte;
    export using ByteBuffer = std::vector<Byte>;
}