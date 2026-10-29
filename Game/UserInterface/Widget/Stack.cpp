module Game.UserInterface.Widget;

import std;
import :Stack;

void Stack::ArrangeChildren()
{
    const bool horizontal = (m_axis == StackAxis::Horizontal);
    const Core::Vector2 size = Size();

    // 교차축에서 자식이 놓일 안쪽 영역
    const float crossStart = horizontal ? m_padding.top : m_padding.left;
    const float crossInner = horizontal ? size.y - m_padding.Vertical()
        : size.x - m_padding.Horizontal();

    float main = horizontal ? m_padding.left : m_padding.top;
    bool first = true;

    for (const auto& child : Children())
    {
        if (!first)
            main += m_spacing;
        first = false;

        const Core::Rect& r = child->LocalRect();
        const float childMain = horizontal ? r.width : r.height;
        const float childCross = horizontal ? r.height : r.width;

        float cross = crossStart;
        switch (m_crossAlign)
        {
        case CrossAlign::Start: break;
        case CrossAlign::Center: cross += std::round((crossInner - childCross) * 0.5f); break; // 홀수 크기의 .5는 정수로 맞춤
        case CrossAlign::End: cross += crossInner - childCross; break;
        }

        SetLocalPosition(*child, horizontal ? Core::Vector2{ main, cross }
        : Core::Vector2{ cross, main });
        main += childMain;
    }
}

Core::Vector2 Stack::MeasureContent() const
{
    const bool horizontal = (m_axis == StackAxis::Horizontal);

    float main = 0.f;    // 주축: 자식 크기의 합
    float cross = 0.f;   // 교차축: 가장 큰 자식
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        main += horizontal ? r.width : r.height;
        cross = (std::max)(cross, horizontal ? r.height : r.width);
    }
    if (!Children().empty())
        main += m_spacing * static_cast<float>(Children().size() - 1);   // 간격은 자식 사이에만

    return horizontal   ? Core::Vector2{ main + m_padding.Horizontal(), cross + m_padding.Vertical() }
                                    : Core::Vector2{ cross + m_padding.Horizontal(), main + m_padding.Vertical() };
}