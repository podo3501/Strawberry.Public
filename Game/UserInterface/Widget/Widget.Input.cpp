module Game.UserInterface.Widget;

import std;
import :Widget;

namespace
{
    bool Contains(const Core::Rect& r, const Core::Vector2& p) noexcept
    {
        return p.x >= r.x && p.x < r.x + r.width
            && p.y >= r.y && p.y < r.y + r.height; // [시작, 끝) 반개구간
    }
}

Widget* Widget::HitTest(const Core::Vector2& screenPos)
{
    return HitTestTree(screenPos, ParentScreenOrigin());
}

Widget* Widget::HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin)
{
    const Core::Rect screen = ScreenRectUnder(parentOrigin);
    const Core::Vector2 childOrigin{ screen.x, screen.y };

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        if (Widget* hit = (*it)->HitTestTree(p, childOrigin))
            return hit;

    return (IsHitTestable() && Contains(screen, p)) ? this : nullptr;
}