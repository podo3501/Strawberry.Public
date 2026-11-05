export module Core.Utils:Hash;

import std;

namespace Core
{
    export template <typename T>
        void HashCombine(std::size_t& seed, const T& value)
    {
        seed ^= std::hash<T>{}(value)
            +0x9e3779b9
            + (seed << 6)
            + (seed >> 2);
    }

    export template <typename... Args>
        std::size_t HashOf(const Args&... args)
    {
        std::size_t seed = 0;

        (HashCombine(seed, args), ...);

        return seed;
    }
}