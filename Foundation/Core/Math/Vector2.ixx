export module Core.Math:Vector2;

import std;
import :Constants;

namespace Core
{
    export struct Vector2
    {
        float x{ 0.0f };
        float y{ 0.0f };

        constexpr Vector2() = default;
        constexpr Vector2(float x, float y) : x(x), y(y) {}

        constexpr Vector2 operator+(const Vector2& rhs) const
        {
            return Vector2(x + rhs.x, y + rhs.y);
        }

        constexpr Vector2 operator-(const Vector2& rhs) const
        {
            return Vector2(x - rhs.x, y - rhs.y);
        }

        constexpr Vector2 operator*(float scalar) const
        {
            return Vector2(x * scalar, y * scalar);
        }

        constexpr Vector2 operator/(float scalar) const
        {
            return Vector2(x / scalar, y / scalar);
        }

        constexpr Vector2& operator+=(const Vector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        constexpr Vector2& operator-=(const Vector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        constexpr bool operator==(const Vector2&) const = default;

        constexpr float LengthSq() const
        {
            return x * x + y * y;
        }

        float Length() const
        {
            return std::sqrt(LengthSq());
        }

        constexpr float Dot(const Vector2& rhs) const
        {
            return x * rhs.x + y * rhs.y;
        }

        Vector2 Normalized() const
        {
            return NormalizedOr(Vector2());
        }

        Vector2 NormalizedOr(const Vector2& fallback) const
        {
            const float len = Length();
            if (len <= NormalizeEpsilon)
                return fallback;

            return *this / len;
        }

        void Normalize()
        {
            *this = NormalizedOr(*this);   // 거의 0이면 그대로 둔다
        }

        void NormalizeOr(const Vector2& fallback)
        {
            *this = NormalizedOr(fallback);
        }

        static constexpr Vector2 Zero() { return Vector2(0.0f, 0.0f); }
        static constexpr Vector2 One() { return Vector2(1.0f, 1.0f); }
    };
}