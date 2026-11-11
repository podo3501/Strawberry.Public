export module Core.Math:Constants;

namespace Core
{
    export constexpr float NormalizeEpsilon = 0.00001f;   // 이 길이 이하의 벡터는 방향이 없는 것으로 본다
    export constexpr float PI = 3.1415926535f;
    export constexpr float Deg2Rad = PI / 180.0f;
    export constexpr float Rad2Deg = 180.0f / PI;
}