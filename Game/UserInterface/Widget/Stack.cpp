module Game.UserInterface.Widget;

import std;
import :Stack;

namespace
{
    float MainOf(const Core::Rect& r, bool horizontal) { return horizontal ? r.width : r.height; }
    float CrossOf(const Core::Rect& r, bool horizontal) { return horizontal ? r.height : r.width; }

    Core::Vector2 Compose(float main, float cross, bool horizontal)
    {
        return horizontal ? Core::Vector2{ main, cross } : Core::Vector2{ cross, main };
    }
}

void Stack::ArrangeChildren()
{
    const bool h = (m_axis == StackAxis::Horizontal);
    const Core::Vector2 size = Size();

    // 교차축에서 자식이 놓일 안쪽 영역
    const float crossStart = h ? m_padding.top : m_padding.left;
    const float crossInner = h ? size.y - m_padding.Vertical() : size.x - m_padding.Horizontal();

    float main = h ? m_padding.left : m_padding.top;
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        const float childCross = CrossOf(r, h);

        float cross = crossStart;
        switch (m_crossAlign)
        {
        case CrossAlign::Start: break;
        case CrossAlign::Center: cross += std::round((crossInner - childCross) * 0.5f); break; // 홀수 크기의 .5는 정수로 맞춤
        case CrossAlign::End: cross += crossInner - childCross; break;
        }

        SetLocalPosition(*child, Compose(main, cross, h));
        main += MainOf(r, h) + m_spacing;   // 간격을 자식 뒤에 붙이면 "첫 자식" 플래그가 필요 없다 (마지막 간격은 쓰이지 않음)
    }
}

Core::Vector2 Stack::MeasureContent() const
{
    const bool h = (m_axis == StackAxis::Horizontal);

    float main = 0.f;    // 주축: 자식 크기의 합
    float cross = 0.f;   // 교차축: 가장 큰 자식
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        main += MainOf(r, h);
        cross = (std::max)(cross, CrossOf(r, h));
    }
    if (!Children().empty())
        main += m_spacing * static_cast<float>(Children().size() - 1);   // 간격은 자식 사이에만

    const Core::Vector2 content = Compose(main, cross, h);
    return { content.x + m_padding.Horizontal(), content.y + m_padding.Vertical() };   // 패딩은 축과 무관하게 x/y에 더한다
}