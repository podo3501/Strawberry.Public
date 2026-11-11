export module Core.Math:MathUtils;

import :Constants;

namespace Core
{
    export constexpr float ToRadians(float degrees)
    {
        return degrees * Deg2Rad;
    }

    export constexpr float ToDegrees(float radians)
    {
        return radians * Rad2Deg;
    }
}