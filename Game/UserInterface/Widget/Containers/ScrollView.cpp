module Game.UserInterface.Widget;

import std;
import :ScrollView;

void ScrollView::SetScroll(float y)
{
    m_scrollY = y;
    ArrangeChildren();
}

bool ScrollView::OnScroll(const ScrollEvent& e)
{
    SetScroll(m_scrollY - e.delta * m_lineStep);
    return true;   // 끝에 닿아도 소비한다. 안쪽 목록 끝에서 바깥 스크롤이 갑자기 움직이는 걸 막는다
}

bool ScrollView::OnPointerDown(const PointerEvent& e)
{
    const auto bar = Bar();
    if (!bar || !bar->track.Contains(e.screenPos.x, e.screenPos.y))
        return false;

    // 손잡이를 잡았으면 잡은 지점을 유지하고, 트랙을 눌렀으면 손잡이 가운데가 포인터에 오도록 점프한다
    m_grab = bar->thumb.Contains(e.screenPos.x, e.screenPos.y)
        ? e.screenPos.y - bar->thumb.y
        : bar->thumb.height * 0.5f;
    m_dragging = true;
    DragTo(e.screenPos.y);
    return true;   // 캡처: 이후 OnPointerMove를 받는다
}

void ScrollView::OnPointerMove(const PointerEvent& e)
{
    if (m_dragging)
        DragTo(e.screenPos.y);
}

void ScrollView::DragTo(float pointerY)
{
    const auto bar = Bar();
    if (!bar || bar->travel <= 0.f)
        return;

    const float maxScroll = m_contentHeight - Size().y;
    const float ratio = std::clamp((pointerY - m_grab - bar->track.y) / bar->travel, 0.f, 1.f);
    SetScroll(ratio * maxScroll);
}

std::optional<ScrollView::BarLayout> ScrollView::Bar() const
{
    const float viewHeight = Size().y;
    const float maxScroll = m_contentHeight - viewHeight;
    if (!m_bar || maxScroll <= 0.f)
        return std::nullopt;

    const Core::Rect view = ScreenRect();
    const Core::Rect track{ view.x + view.width - m_bar->width, view.y, m_bar->width, view.height };

    // 길이도 정수 픽셀로 맞춘다. 그래야 맨 아래에서 손잡이 끝이 트랙 끝에 정확히 닿는다
    const float length = std::clamp(std::round(viewHeight * viewHeight / m_contentHeight), (std::min)(m_bar->minThumbLength, viewHeight), viewHeight);
    const float travel = viewHeight - length;
    const float top = track.y + std::round(travel * (m_scrollY / maxScroll));

    return BarLayout{ track, Core::Rect{ track.x, top, track.width, length }, travel };
}

void ScrollView::DrawAfterChildren(IOverlayView& overlay, const Core::Rect&) const
{
    if (const auto bar = Bar())
    {
        overlay.DrawImage(m_bar->track, bar->track);
        overlay.DrawImage(m_bar->thumb, bar->thumb);
    }
}

void ScrollView::ArrangeChildren()
{
    // 앵커 배치를 그대로 재사용한다. 위치는 매번 앵커에서 새로 계산되므로 아래의 이동이 누적되지 않는다.
    Widget::ArrangeChildren();

    float contentBottom = 0.f;
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        contentBottom = (std::max)(contentBottom, r.y + r.height);
    }

    m_contentHeight = contentBottom;
    const float maxScroll = (std::max)(0.f, contentBottom - Size().y);
    m_scrollY = std::clamp(m_scrollY, 0.f, maxScroll);   // 내용이 줄거나 뷰포트가 커져도 여기서 되돌아온다

    const float shift = std::round(m_scrollY);   // 반 픽셀에 걸려 글자와 이미지가 흐려지는 걸 막는다 (Anchor::Resolve와 같은 이유)
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        SetLocalPosition(*child, { r.x, r.y - shift });
    }
}
