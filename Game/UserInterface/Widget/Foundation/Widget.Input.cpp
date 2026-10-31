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

    // 자식을 자르는 위젯은 잘려서 보이지 않는 영역의 클릭도 막는다 (그리기의 PushClip과 같은 영역).
    // 자르지 않는 위젯은 자식이 부모 rect 밖에 그려지면 그 부분도 히트 대상이다. 툴팁, 드롭다운처럼 부모 밖으로 튀어나오는 위젯을 위한 의도된 동작이다.
    if (ClipsChildren() && !screen.Contains(p.x, p.y))
        return nullptr;

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        if (Widget* hit = (*it)->HitTestTree(p, childOrigin))
            return hit;

    return (IsHitTestable() && screen.Contains(p.x, p.y)) ? this : nullptr;
}