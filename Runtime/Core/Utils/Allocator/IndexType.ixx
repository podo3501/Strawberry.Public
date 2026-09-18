export module Core.Types:Allocator;

import std;

namespace Core
{
    export using Index = std::uint32_t;
    export constexpr Index InvalidIndex = std::numeric_limits<Index>::max();
}