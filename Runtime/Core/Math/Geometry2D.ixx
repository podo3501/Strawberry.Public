export module Core.Math:Geometry2D;

import std;
import Core.Assert;

namespace Core
{
    export struct Point
    {
        int x{ 0 };
        int y{ 0 };

        bool operator==(const Point&) const = default;
    };

    export inline Point ToPoint(int x, int y) { return { x, y }; }

    export struct Size
    {
        std::uint32_t width{ 0 };
        std::uint32_t height{ 0 };

        constexpr Size() noexcept = default;
        constexpr Size(std::uint32_t w, std::uint32_t h) noexcept : width{ w }, height{ h } {}

        bool operator==(const Size&) const = default;
    };

    export constexpr Size ToSize(int width, int height)
    {
        Assert(width >= 0 && height >= 0);
        return Size{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

    export constexpr Size ToSize(float width, float height)
    {
        Assert(width >= 0.f && height >= 0.f);
        return Size{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

    export struct Rect
    {
        float x{ 0.f }, y{ 0.f }, width{ 0.f }, height{ 0.f };

        bool IsValid() const { return width > 0.f && height > 0.f; }

        float Left() const { return x; }
        float Right() const { return x + width; }
        float Top() const { return y; }
        float Bottom() const { return y + height; }

        bool Contains(float px, float py) const
        {
            Assert(width >= 0.f && height >= 0.f);

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

        bool Contains(const Rect& other) const
        {
            return other.x >= x &&
                other.y >= y &&
                other.x + other.width <= x + width &&
                other.y + other.height <= y + height;
        }

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

        bool Intersects(const Rect& other) const
        {
            return !(other.Left() >= Right() ||
                other.Right() <= Left() ||
                other.Top() >= Bottom() ||
                other.Bottom() <= Top());
        }

        Rect Intersect(const Rect& other) const
        {
            float l1 = x;
            float t1 = y;
            float r1 = x + width;
            float b1 = y + height;

            float l2 = other.x;
            float t2 = other.y;
            float r2 = other.x + other.width;
            float b2 = other.y + other.height;

            float l = std::max(l1, l2);
            float t = std::max(t1, t2);
            float r = std::min(r1, r2);
            float b = std::min(b1, b2);

            return { l, t, r - l, b - t };
        }
    };
}