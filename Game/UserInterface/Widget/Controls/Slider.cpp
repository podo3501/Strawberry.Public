module Game.UserInterface.Widget;

import std;
import :Slider;

Slider::Slider(Core::Rect localRect, SliderSkin skin, SliderAxis axis, float value)
    : Widget(localRect)
    , m_skin(std::move(skin))
    , m_axis(axis)
    , m_value(Clamp01(value)) 
{}

bool Slider::OnPointerDown(const PointerEvent& e)
{
    SetFromPointer(e.screenPos);
    return true;   // 캡처: 이후 OnPointerMove를 받는다
}

void Slider::OnPointerMove(const PointerEvent& e)
{
    SetFromPointer(e.screenPos);
}

void Slider::SetFromPointer(const Core::Vector2& p)
{
    const Core::Rect r = ScreenRect();
    const Core::Vector2& thumb = m_skin.thumbSize;

    // 값 0일 때 썸 중심에서 포인터까지 주축으로 얼마나 왔는가 (세로는 아래에서 위로)
    const float along = IsHorizontal()
        ? p.x - (r.x + thumb.x * 0.5f)
        : (r.y + r.height - thumb.y * 0.5f) - p.y;

    const float travel = Travel();
    const float value = travel > 0.f ? std::clamp(along / travel, 0.f, 1.f) : 0.f;

    if (value == m_value)
        return;

    m_value = value;
    if (m_onChanged)
        m_onChanged(m_value);   // 콜백이 위젯을 파괴할 수 있으니 마지막에 부른다
}

void Slider::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    const Core::Vector2& thumb = m_skin.thumbSize;
    const float shift = Travel() * m_value;

    DrawGauge(overlay, m_skin.bar, screenRect, m_value,
        IsHorizontal() ? GaugeDirection::LeftToRight : GaugeDirection::BottomToTop);

    const Core::Vector2 pos = IsHorizontal()
        ? Core::Vector2{ screenRect.x + shift, screenRect.y + (screenRect.height - thumb.y) * 0.5f }
    : Core::Vector2{ screenRect.x + (screenRect.width - thumb.x) * 0.5f, screenRect.y + screenRect.height - thumb.y - shift };

    overlay.DrawImage(m_skin.thumb, { std::round(pos.x), std::round(pos.y), thumb.x, thumb.y });
}