export module Core.Math:Vector2;

import std; // 표준 라이브러리 모듈 전체 가져오기

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

        constexpr bool operator==(const Vector2& rhs) const
        {
            return x == rhs.x && y == rhs.y;
        }

        constexpr bool operator!=(const Vector2& rhs) const
        {
            return !(*this == rhs);
        }

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

        float Distance(const Vector2& rhs) const
        {
            return (*this - rhs).Length();
        }

        constexpr float DistanceSq(const Vector2& rhs) const
        {
            return (*this - rhs).LengthSq();
        }

        Vector2 Normalized() const
        {
            float len = Length();
            if (len <= 0.00001f)
                return Vector2(0.0f, 0.0f);

            return *this / len;
        }

        Vector2 NormalizedOr(const Vector2& fallback) const
        {
            float len = Length();
            if (len <= 0.00001f)
                return fallback;

            return *this / len;
        }

        void Normalize()
        {
            float len = Length();
            if (len <= 0.00001f)
                return;

            x /= len;
            y /= len;
        }

        void NormalizeOr(const Vector2& fallback)
        {
            float len = Length();
            if (len <= 0.00001f)
            {
                *this = fallback;
                return;
            }

            x /= len;
            y /= len;
        }

        static constexpr Vector2 Zero() { return Vector2(0.0f, 0.0f); }
        static constexpr Vector2 One() { return Vector2(1.0f, 1.0f); }
    };
}