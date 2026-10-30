module Game.UserInterface.Widget;

import std;
import :Widget;

Widget::Widget(Core::Rect localRect) // 처음 주어진 x, y는 TopLeft 앵커의 오프셋이 된다
    : m_rect(localRect)
    , m_anchor(Anchor::TopLeft().WithOffset(localRect.x, localRect.y))
{}

std::weak_ptr<Widget*> Widget::Handle()   // 호버/눌림 추적에 쓰일 때만 만든다.
{
    if (!m_handle)
        m_handle = std::make_shared<Widget*>(this);
    return m_handle;
}

// parentOrigin 기준으로 이 위젯의 화면 rect를 구한다. 그리기와 hit test가 공유하는 유일한 변환
Core::Rect Widget::ScreenRectUnder(const Core::Vector2& parentOrigin) const
{
    return { parentOrigin.x + m_rect.x, parentOrigin.y + m_rect.y, m_rect.width, m_rect.height };
}

Core::Vector2 Widget::ParentScreenOrigin() const
{
    Core::Vector2 o;
    for (const Widget* w = m_parent; w; w = w->m_parent)
        o = o + Core::Vector2{ w->m_rect.x, w->m_rect.y };
    return o;
}

void Widget::Draw(IOverlayView& overlay) const
{
    DrawTree(overlay, ParentScreenOrigin());
}

void Widget::DrawTree(IOverlayView& overlay, const Core::Vector2& parentOrigin) const
{
    const Core::Rect screen = ScreenRectUnder(parentOrigin);

    DrawSelf(overlay, screen);   // 자기 자신은 클립 밖에서 그린다 (배경이 자기 rect를 넘을 일은 없지만, 부모의 클립만 받는다)

    const bool clips = ClipsChildren();
    if (clips)
        overlay.PushClip(screen);

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (const auto& child : m_children)
        child->DrawTree(overlay, childOrigin);

    if (clips)
        overlay.PopClip();
}