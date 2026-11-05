export module Core.Math:Utils;

import :Constants;

namespace Core
{
    export constexpr float ToRadians(float degrees)
    {
        return degrees * (PI / 180.0f); // 또는 degrees * Deg2Rad;
    }

    export constexpr float ToDegrees(float radians)
    {
        return radians * (180.0f / PI); // 또는 radians * Rad2Deg;
    }
}