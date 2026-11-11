export module Core.Math:Geometry2D;

import std;

namespace Core
{
    export struct Point
    {
        int x{ 0 };
        int y{ 0 };

        bool operator==(const Point&) const = default;
    };

    export struct Size
    {
        std::uint32_t width{ 0 };
        std::uint32_t height{ 0 };

        constexpr Size() noexcept = default;
        constexpr Size(std::uint32_t w, std::uint32_t h) noexcept : width{ w }, height{ h } {}

        bool operator==(const Size&) const = default;
    };

    // 크기는 음수가 될 수 없으므로 음수는 0이다
    constexpr std::uint32_t ToSizeValue(int value)
    {
        return value > 0 ? static_cast<std::uint32_t>(value) : 0u;
    }

    // 음수와 NaN은 0, uint32 범위를 넘으면 최댓값이다. 소수는 버린다
    constexpr std::uint32_t ToSizeValue(float value)
    {
        if (!(value > 0.f))
            return 0u;
        if (value >= 4294967296.f)
            return (std::numeric_limits<std::uint32_t>::max)();

        return static_cast<std::uint32_t>(value);
    }

    export constexpr Size ToSize(int width, int height)
    {
        return Size{ ToSizeValue(width), ToSizeValue(height) };
    }

    export constexpr Size ToSize(float width, float height)
    {
        return Size{ ToSizeValue(width), ToSizeValue(height) };
    }

    export struct Rect
    {
        bool operator==(const Rect&) const = default;

        float x{ 0.f }, y{ 0.f }, width{ 0.f }, height{ 0.f };

        bool IsValid() const { return width > 0.f && height > 0.f; }

        float Left() const { return x; }
        float Right() const { return x + width; }
        float Top() const { return y; }
        float Bottom() const { return y + height; }

        // 크기가 0이거나 음수이면 비어 있어서 아무것도 포함하지 않는다
        bool Contains(float px, float py) const
        {
            return px >= x &&
                px < x + width &&
                py >= y &&
                py < y + height;
        }

        bool Contains(const Point& p) const
        {
            return Contains(
                static_cast<float>(p.x),
                static_cast<float>(p.y));
        }

        // 빈 사각형(크기가 0 이하)은 아무것도 포함하지 않고 어디에도 포함되지 않는다
        bool Contains(const Rect& other) const
        {
            return IsValid() && other.IsValid() &&
                other.x >= x &&
                other.y >= y &&
                other.x + other.width <= x + width &&
                other.y + other.height <= y + height;
        }

        // 음수 크기를 뒤집어서 같은 영역의 양수 크기 사각형으로 만든다
        void Normalize()
        {
            if (width < 0.f)
            {
                x += width;
                width = -width;
            }

            if (height < 0.f)
            {
                y += height;
                height = -height;
            }
        }

        // 빈 사각형(크기가 0 이하)은 아무것과도 겹치지 않는다
        bool Intersects(const Rect& other) const
        {
            return IsValid() && other.IsValid() &&
                !(other.Left() >= Right() ||
                other.Right() <= Left() ||
                other.Top() >= Bottom() ||
                other.Bottom() <= Top());
        }

        Rect Intersect(const Rect& other) const
        {
            const float l = (std::max)(x, other.x);
            const float t = (std::max)(y, other.y);
            const float r = (std::min)(Right(), other.Right());
            const float b = (std::min)(Bottom(), other.Bottom());

            return { l, t, (std::max)(0.f, r - l), (std::max)(0.f, b - t) };   // 겹치지 않으면 크기 0
        }
    };
}