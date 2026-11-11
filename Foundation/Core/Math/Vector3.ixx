export module Core.Math:Vector3;

import std;
import :Constants;

namespace Core
{
    export struct Vector3
    {
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };

        constexpr Vector3() = default;
        constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        constexpr Vector3 operator+(const Vector3& rhs) const
        {
            return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
        }

        constexpr Vector3 operator-(const Vector3& rhs) const
        {
            return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        constexpr Vector3 operator*(float scalar) const
        {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }

        constexpr Vector3 operator/(float scalar) const
        {
            return Vector3(x / scalar, y / scalar, z / scalar);
        }

        constexpr Vector3& operator+=(const Vector3& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        constexpr Vector3& operator-=(const Vector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        constexpr bool operator==(const Vector3&) const = default;

        constexpr float LengthSq() const
        {
            return x * x + y * y + z * z;
        }

        float Length() const
        {
            return std::sqrt(LengthSq());
        }

        constexpr float Dot(const Vector3& rhs) const
        {
            return x * rhs.x + y * rhs.y + z * rhs.z;
        }

        constexpr Vector3 Cross(const Vector3& rhs) const
        {
            return Vector3(
                y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x
            );
        }

        Vector3 Normalized() const
        {
            return NormalizedOr(Vector3());
        }

        Vector3 NormalizedOr(const Vector3& fallback) const
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

        void NormalizeOr(const Vector3& fallback)
        {
            *this = NormalizedOr(fallback);
        }

        static constexpr Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
        static constexpr Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }
        static constexpr Vector3 Up() { return Vector3(0.0f, 1.0f, 0.0f); }
        static constexpr Vector3 Right() { return Vector3(1.0f, 0.0f, 0.0f); }
        static constexpr Vector3 Forward() { return Vector3(0.0f, 0.0f, 1.0f); }
    };
}