module Game.UserInterface.Widget;

import std;
import :Widget;

Widget::Widget(Core::Rect localRect) // 처음 주어진 x, y는 TopLeft 앵커의 오프셋이 된다
    : m_rect(localRect)
    , m_anchor(Anchor::TopLeft().WithOffset(localRect.x, localRect.y))
{}

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

    DrawSelf(overlay, screen);

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (const auto& child : m_children)
        child->DrawTree(overlay, childOrigin);   // 같은 클래스라 private 접근 가능
}