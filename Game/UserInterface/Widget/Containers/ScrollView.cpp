module Game.UserInterface.Widget;

import std;
import :ScrollView;

void ScrollView::SetScroll(float y)
{
    m_scrollY = y;
    InvalidateLayout();   // 배치 안에서 범위를 맞춘다
}

bool ScrollView::OnScroll(const ScrollEvent& e)
{
    SetScroll(m_scrollY - e.delta * m_lineStep);
    return true;   // 끝에 닿아도 소비한다. 안쪽 목록 끝에서 바깥 스크롤이 갑자기 움직이는 걸 막는다
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

    const float maxScroll = (std::max)(0.f, contentBottom - Size().y);
    m_scrollY = std::clamp(m_scrollY, 0.f, maxScroll);   // 내용이 줄거나 뷰포트가 커져도 여기서 되돌아온다

    const float shift = std::round(m_scrollY);   // 반 픽셀에 걸려 글자와 이미지가 흐려지는 걸 막는다 (Anchor::Resolve와 같은 이유)
    for (const auto& child : Children())
    {
        const Core::Rect& r = child->LocalRect();
        SetLocalPosition(*child, { r.x, r.y - shift });
    }
}