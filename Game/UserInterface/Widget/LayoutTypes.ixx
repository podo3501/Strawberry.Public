export module Game.UserInterface.Widget:LayoutTypes;

import std;
import Core.Math;

export enum class SizePolicy // Fit: 내용 크기에 맞춤 (소수는 올림)
{ 
    Fixed, 
    Fit 
};   

export struct Anchor
{
    Core::Vector2 anchor{ 0.f, 0.f };  // 부모 rect 안의 기준점 (0~1) 
    Core::Vector2 pivot{ 0.f, 0.f };  // 자기 rect 안의 기준점 (0~1)
    Core::Vector2 offset{ 0.f, 0.f };  // 픽셀 오프셋 (+x 오른쪽, +y 아래)
    //anchor는 어디에 붙일지, pivot은 무엇을 붙일지, offset은 얼마나 이동할지를 결정

    // anchor와 pivot이 같은 점인 프리셋의 공통 생성(anchor와 pivot이 같을때가 종종 있기 때문에)
    static constexpr Anchor At(float x, float y) noexcept
    {
        return { { x, y },{ x, y },{ 0.f, 0.f } };
    }

    constexpr Anchor WithOffset(float x, float y) const noexcept
    {
        Anchor anchor = *this;
        anchor.offset = { x, y };
        return anchor;
    }

    [[nodiscard]] Core::Vector2 Resolve(const Core::Vector2& parentSize, const Core::Vector2& selfSize) const noexcept
    {
        return {
            std::round(parentSize.x * anchor.x - selfSize.x * pivot.x + offset.x),
            std::round(parentSize.y * anchor.y - selfSize.y * pivot.y + offset.y)
        };
    }

    static constexpr Anchor TopLeft() noexcept { return At(0.0f, 0.0f); }
    static constexpr Anchor Top() noexcept { return At(0.5f, 0.0f); }
    static constexpr Anchor TopRight() noexcept { return At(1.0f, 0.0f); }
    static constexpr Anchor Left() noexcept { return At(0.0f, 0.5f); }
    static constexpr Anchor Center() noexcept { return At(0.5f, 0.5f); }
    static constexpr Anchor Right() noexcept { return At(1.0f, 0.5f); }
    static constexpr Anchor BottomLeft() noexcept { return At(0.0f, 1.0f); }
    static constexpr Anchor Bottom() noexcept { return At(0.5f, 1.0f); }
    static constexpr Anchor BottomRight() noexcept { return At(1.0f, 1.0f); }
};

export struct Padding
{
    float left{ 0.f };
    float top{ 0.f };
    float right{ 0.f };
    float bottom{ 0.f };

    static constexpr Padding All(float v) noexcept { return { v, v, v, v }; }
    static constexpr Padding Symmetric(float horizontal, float vertical) noexcept
    {
        return { horizontal, vertical, horizontal, vertical };
    }

    constexpr float Horizontal() const noexcept { return left + right; }
    constexpr float Vertical() const noexcept { return top + bottom; }
};