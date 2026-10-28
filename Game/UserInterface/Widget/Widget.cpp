module Game.UserInterface.Widget;

import std;
import :Widget;
import Core.Assert;

std::unique_ptr<Widget> Widget::Detach()
{
    Core::Assert(m_parent && "Detach: 부모가 없는 위젯");

    auto& siblings = m_parent->m_children;
    auto it = std::find_if(siblings.begin(), siblings.end(),
        [this](const std::unique_ptr<Widget>& p) { return p.get() == this; });
    Core::Assert(it != siblings.end());

    std::unique_ptr<Widget> self = std::move(*it);
    siblings.erase(it);
    m_parent = nullptr;
    return self;
}

void Widget::Draw(IOverlayView& overlay) const
{
    DrawTree(overlay, ParentScreenOrigin());
}

Widget* Widget::HitTest(const Core::Vector2& screenPos)
{
    return HitTestTree(screenPos, ParentScreenOrigin());
}

Core::Vector2 Widget::ParentScreenOrigin() const
{
    Core::Vector2 o;
    for (const Widget* w = m_parent; w; w = w->m_parent)
        o = o + Core::Vector2{ w->m_rect.x, w->m_rect.y };
    return o;
}

void Widget::AttachImpl(std::unique_ptr<Widget> child, std::size_t index)
{
    Core::Assert(child); // Attach: null 위젯
    Core::Assert(!child->m_parent); // Attach: 이미 부모가 있음. Detach 먼저.
    Core::Assert(!IsDescendantOf(*child)); // Attach: 자기 자신/자손 밑에 붙이면 순환

    child->m_parent = this;
    index = (std::min)(index, m_children.size());
    m_children.insert(m_children.begin() + static_cast<std::ptrdiff_t>(index),
        std::move(child));
}

bool Widget::IsDescendantOf(const Widget& ancestor) const
{
    for (const Widget* w = this; w; w = w->m_parent)
        if (w == &ancestor) return true;
    return false;
}

void Widget::DrawTree(IOverlayView& overlay, const Core::Vector2& origin) const
{
    const Core::Rect screen{
        origin.x + m_rect.x, origin.y + m_rect.y, m_rect.width, m_rect.height };

    DrawSelf(overlay, screen);

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (const auto& child : m_children)
        child->DrawTree(overlay, childOrigin);   // 같은 클래스라 private 접근 가능
}

Widget* Widget::HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin)
{
    const Core::Vector2 origin{ parentOrigin.x + m_rect.x, parentOrigin.y + m_rect.y };

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        if (Widget* hit = (*it)->HitTestTree(p, origin))
            return hit;

    if (!IsHitTestable())
        return nullptr;

    const bool inside = p.x >= origin.x && p.x < origin.x + m_rect.width
        && p.y >= origin.y && p.y < origin.y + m_rect.height; // [시작, 끝) 반개구간
    return inside ? this : nullptr;
}