export module Core.Color;

import std;
import Core.Vector4; // Vector4 타입 모듈 import

namespace Core
{
    export struct Color
    {
        float r{ 1.0f };
        float g{ 1.0f };
        float b{ 1.0f };
        float a{ 1.0f };

        constexpr Color() = default;

        constexpr Color(float r, float g, float b, float a = 1.0f)
            : r(r), g(g), b(b), a(a)
        {
        }

        constexpr explicit Color(const Vector4& v)
            : r(v.x), g(v.y), b(v.z), a(v.w)
        {
        }

        constexpr operator Vector4() const
        {
            return { r, g, b, a };
        }

        [[nodiscard]]
        constexpr Color WithAlpha(float alpha) const
        {
            return { r, g, b, alpha };
        }

        [[nodiscard]]
        static constexpr Color FromARGB8888(std::uint32_t argb)
        {
            return
            {
                ((argb >> 16) & 0xFF) / 255.0f,
                ((argb >> 8) & 0xFF) / 255.0f,
                (argb & 0xFF) / 255.0f,
                ((argb >> 24) & 0xFF) / 255.0f
            };
        }

        [[nodiscard]]
        static constexpr Color FromRGBA8888(std::uint32_t rgba)
        {
            return
            {
                ((rgba >> 24) & 0xFF) / 255.0f,
                ((rgba >> 16) & 0xFF) / 255.0f,
                ((rgba >> 8) & 0xFF) / 255.0f,
                (rgba & 0xFF) / 255.0f
            };
        }

        constexpr bool operator==(const Color&) const = default;
        constexpr bool operator!=(const Color&) const = default;

        constexpr std::uint8_t R8() const { return static_cast<std::uint8_t>(r * 255.0f + 0.5f); }
        constexpr std::uint8_t G8() const { return static_cast<std::uint8_t>(g * 255.0f + 0.5f); }
        constexpr std::uint8_t B8() const { return static_cast<std::uint8_t>(b * 255.0f + 0.5f); }
        constexpr std::uint8_t A8() const { return static_cast<std::uint8_t>(a * 255.0f + 0.5f); }
    };

    export namespace Colors
    {
        inline constexpr Color White{ 1.0f, 1.0f, 1.0f, 1.0f };
        inline constexpr Color Black{ 0.0f, 0.0f, 0.0f, 1.0f };
        inline constexpr Color Red{ 1.0f, 0.0f, 0.0f, 1.0f };
        inline constexpr Color Green{ 0.0f, 1.0f, 0.0f, 1.0f };
        inline constexpr Color Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
        inline constexpr Color Yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
        inline constexpr Color Cyan{ 0.1f, 1.0f, 1.0f, 1.0f };
        inline constexpr Color Magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
        inline constexpr Color Gray{ 0.5f, 0.5f, 0.5f, 1.0f };
        inline constexpr Color Transparent{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    static_assert(sizeof(Color) == sizeof(Vector4));
    static_assert(alignof(Color) == alignof(Vector4));
}