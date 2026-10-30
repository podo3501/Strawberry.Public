module Game.UserInterface.Widget;

import std;
import :Widget;

Widget* Widget::HitTest(const Core::Vector2& screenPos)
{
    return HitTestTree(screenPos, ParentScreenOrigin());
}

Widget* Widget::HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin)
{
    const Core::Rect screen = ScreenRectUnder(parentOrigin);

    // 자식을 자르는 위젯은 잘려서 안 보이는 영역의 클릭도 막는다.
    // 자르지 않는 위젯은 기존대로 부모 rect 밖의 자식도 히트 대상이다.
    if (ClipsChildren() && !screen.Contains(p.x, p.y))
        return nullptr;

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        if (Widget* hit = (*it)->HitTestTree(p, childOrigin))
            return hit;

    return (IsHitTestable() && screen.Contains(p.x, p.y)) ? this : nullptr;
}