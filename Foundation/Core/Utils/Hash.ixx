export module Core.Utils:Hash;

import std;

namespace Core
{
    // 황금비 상수로 섞는다 (std::size_t가 64비트이므로 64비트 상수다)
    export template <typename T>
    void HashCombine(std::size_t& seed, const T& value)
    {
        static_assert(sizeof(std::size_t) == 8, "상수가 64비트용이다");
        seed ^= std::hash<T>{}(value)
            + 0x9e3779b97f4a7c15ULL
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