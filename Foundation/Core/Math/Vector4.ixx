export module Core.Math:Vector4;

import std;
import :Vector3;

namespace Core
{
    export struct Vector4
    {
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };
        float w{ 0.0f };

        constexpr Vector4() = default;
        constexpr Vector4(float inX, float inY, float inZ, float inW)
            : x(inX), y(inY), z(inZ), w(inW) {
        }

        constexpr Vector4(const Vector3& v3, float inW)
            : x(v3.x), y(v3.y), z(v3.z), w(inW) {
        }

        constexpr Vector4 operator+(const Vector4& rhs) const
        {
            return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
        }

        constexpr Vector4 operator-(const Vector4& rhs) const
        {
            return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
        }

        constexpr Vector4 operator*(float s) const
        {
            return Vector4(x * s, y * s, z * s, w * s);
        }

        constexpr Vector4 operator/(float s) const
        {
            return Vector4(x / s, y / s, z / s, w / s);
        }

        constexpr Vector4& operator+=(const Vector4& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        constexpr Vector4& operator-=(const Vector4& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        constexpr bool operator==(const Vector4&) const = default;

        constexpr float LengthSq() const
        {
            return x * x + y * y + z * z + w * w;
        }

        float Length() const
        {
            return std::sqrt(LengthSq());
        }

        constexpr float Dot(const Vector4& rhs) const
        {
            return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
        }

        static constexpr Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Vector4 One() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
    };
}