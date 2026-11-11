export module Core.Math:Color;

import std;
import :Vector4;

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

        // RGB를 sRGB 값에서 선형 값으로 바꾼다. 알파는 그대로다 (1.0은 1.0, 1을 넘는 값은 넘는 대로 이어진다)
        [[nodiscard]]
        Color SrgbToLinear() const
        {
            return { SrgbChannelToLinear(r), SrgbChannelToLinear(g), SrgbChannelToLinear(b), a };
        }

        constexpr bool operator==(const Color&) const = default;

        // 0~1을 벗어난 값은 0 또는 255로 제한한다 (NaN은 0). 틴트처럼 1을 넘는 값도 들어올 수 있다
        constexpr std::uint8_t R8() const { return ToByte(r); }
        constexpr std::uint8_t G8() const { return ToByte(g); }
        constexpr std::uint8_t B8() const { return ToByte(b); }
        constexpr std::uint8_t A8() const { return ToByte(a); }

        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Cyan;
        static const Color Magenta;
        static const Color Gray;
        static const Color Transparent;

    private:
        static constexpr std::uint8_t ToByte(float value)
        {
            if (!(value > 0.0f))
                return 0;
            if (value >= 1.0f)
                return 255;

            return static_cast<std::uint8_t>(value * 255.0f + 0.5f);
        }

        static float SrgbChannelToLinear(float c)
        {
            return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
        }
    };

    inline constexpr Color Color::White{ 1.0f, 1.0f, 1.0f, 1.0f };
    inline constexpr Color Color::Black{ 0.0f, 0.0f, 0.0f, 1.0f };
    inline constexpr Color Color::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
    inline constexpr Color Color::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
    inline constexpr Color Color::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
    inline constexpr Color Color::Yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
    inline constexpr Color Color::Cyan{ 0.0f, 1.0f, 1.0f, 1.0f };
    inline constexpr Color Color::Magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
    inline constexpr Color Color::Gray{ 0.5f, 0.5f, 0.5f, 1.0f };
    inline constexpr Color Color::Transparent{ 0.0f, 0.0f, 0.0f, 0.0f };

    static_assert(sizeof(Color) == sizeof(Vector4));
    static_assert(alignof(Color) == alignof(Vector4));
}